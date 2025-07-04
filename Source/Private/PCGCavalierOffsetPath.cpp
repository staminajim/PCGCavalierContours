// Copyright 2025 James Vanas
// MIT License – see LICENSE file for details.

#include "PCGCavalierOffsetPath.h"

#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGSpatialData.h"
#include "Data/PCGBasePointData.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGAsync.h"
#include "Helpers/PCGHelpers.h"

#include "Math/RandomStream.h"

THIRD_PARTY_INCLUDES_START
#include "polylineoffset.hpp"
THIRD_PARTY_INCLUDES_END

#include UE_INLINE_GENERATED_CPP_BY_NAME(PCGCavalierOffsetPath)

#define LOCTEXT_NAMESPACE "PCGCavalierOffsetPathElement"

FPCGElementPtr UPCGCavalierOffsetPathSettings::CreateElement() const
{
	return MakeShared<FPCGCavalierOffsetPathElement>();
}

bool FPCGCavalierOffsetPathElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGCavalierOffsetPathElement::Execute);
	
	const UPCGCavalierOffsetPathSettings* Settings = Context->GetInputSettings<UPCGCavalierOffsetPathSettings>();
	check(Settings);
	
	const TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	
	const double ParallelOffset = Settings->bNegateOffset ? -Settings->Offset : Settings->Offset;	
	const bool bClosedPath = Settings->bClosedPath;
	const bool bRecomputeSeed = Settings->bRecomputeSeed;
	
	
	
	const int Seed = Context->GetSeed();
	
	auto FindNearestInputPoint = [](const TArray<FPCGPoint>& InputPoints, double X, double Y) -> int32
	{
		int32 NearestIndex = 0;
		double NearestDistSq = FLT_MAX;
		
		for (int32 i = 0; i < InputPoints.Num(); ++i)
		{
			FVector InputLocation = InputPoints[i].Transform.GetLocation();
			const double DistSq = FMath::Square(InputLocation.X - X) + FMath::Square(InputLocation.Y - Y);
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestIndex = i;
			}
		}
		
		return NearestIndex;
	};
	
	for (const FPCGTaggedData& Input : Inputs)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FPCGCavalierOffsetPathElement::Execute::InputLoop);
		
		const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);
		
		if (!SpatialData)
		{
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingSpatialData", "Unable to get Spatial data from input"));
			continue;
		}
		
		const UPCGPointData* PointData = SpatialData->ToPointData(Context);		
		
		if (!PointData)
		{
			PCGE_LOG(Error, GraphAndLog, LOCTEXT("InputMissingPointData", "Unable to get Point data from input"));
			continue;
		}
		
		TArray<FPCGPoint> InputPoints;
		InputPoints.Reserve(PointData->GetNumPoints());
		const TArray<FPCGPoint>& Points = PointData->GetPoints();
		for (int32 i = 0; i < PointData->GetNumPoints(); ++i)
		{
			InputPoints.Add(Points[i]);
		}
		
		cavc::Polyline<double> InputPolyline;
		for (const FPCGPoint& Point : InputPoints)
		{
			FVector Location = Point.Transform.GetLocation();
			InputPolyline.addVertex(Location.X, Location.Y, 0.0);
		}
		InputPolyline.isClosed() = bClosedPath;
		
		std::vector<cavc::Polyline<double>> OffsetResults = cavc::parallelOffset(InputPolyline, ParallelOffset);
		
		for (int32 PolylineIndex = 0; PolylineIndex < OffsetResults.size(); ++PolylineIndex)
		{
			const cavc::Polyline<double>& ResultPolyline = OffsetResults[PolylineIndex];
			
			if (ResultPolyline.size() == 0) continue;
			
			FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
			UPCGPointData* OutputData = FPCGContext::NewObject_AnyThread<UPCGPointData>(Context);
			OutputData->InitializeFromData(PointData);
			Output.Data = OutputData;
			
			const TArray<FPCGPoint>& InputPointsArray = PointData->GetPoints();
			TArray<FPCGPoint> OutputPointsArray;
			OutputPointsArray.Reserve(ResultPolyline.size());
			
			for (int32 VertexIndex = 0; VertexIndex < ResultPolyline.size(); ++VertexIndex)
			{
				const auto& Vertex = ResultPolyline[VertexIndex];
				
				int32 NearestInputIndex = FindNearestInputPoint(InputPoints, Vertex.x(), Vertex.y());
				FPCGPoint OutputPoint = InputPointsArray[NearestInputIndex];
				
				FVector NewLocation(Vertex.x(), Vertex.y(), OutputPoint.Transform.GetLocation().Z);
				OutputPoint.Transform.SetLocation(NewLocation);
				
				if (bRecomputeSeed)
				{
					OutputPoint.Seed = PCGHelpers::ComputeSeed((int)NewLocation.X, (int)NewLocation.Y, (int)NewLocation.Z);
				}
				
				OutputPointsArray.Add(OutputPoint);
			}
			
			OutputData->SetPoints(OutputPointsArray);
		}
	}
	
	return true;
}

#undef LOCTEXT_NAMESPACE
