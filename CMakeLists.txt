cmake_minimum_required(VERSION 3.15)
project(FrequencyDP)

SET(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)

add_executable(FrequencyDP main.cpp commonheader.h Parameters.cpp Parameters.h Utils.h
        io/FileReader.cpp io/FileReader.h io/LogFile.cpp io/LogFile.h io/FileOutput.h
        grids/Grid.cpp grids/Grid.h grids/StaticGrid.cpp grids/StaticGrid.h
        spatial/GeoPoint.cpp spatial/GeoPoint.h spatial/Point.cpp spatial/Point.h spatial/Trip.cpp spatial/Trip.h
        linking/FrequencyVec.cpp linking/FrequencyVec.h linking/Signature.cpp linking/Signature.h linking/Functions.h
        privacy/NoiseGenerator.h privacy/EditOperation.h privacy/PerturbDistribution.h
        privacy/HierarGrid.cpp privacy/HierarGrid.h privacy/GridCell.cpp privacy/GridCell.h privacy/Segment.cpp privacy/Segment.h
        privacy/global/GlobalMechanism.h privacy/global/GlobalEdit.cpp privacy/global/GlobalEdit.h privacy/global/GlobalHGrid.cpp privacy/global/GlobalHGrid.h
        privacy/global/gGridCell.cpp privacy/global/gGridCell.h privacy/global/gSegment.cpp privacy/global/gSegment.h
        privacy/local/LocalMechanism.h privacy/local/LocalEdit.cpp privacy/local/LocalEdit.h privacy/local/LocalHGrid.cpp privacy/local/LocalHGrid.h
        privacy/local/lGridCell.cpp privacy/local/lGridCell.h privacy/local/lSegment.cpp privacy/local/lSegment.h)
