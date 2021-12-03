ICDE 2022, Paper 916 submission
## Frequency-based Randomization for Guaranteeing Differential Privacy in Spatial Trajectories
#### Authors: Fengmei Jin, Wen Hua, Boyu Ruan, Xiaofang Zhou

- A novel differential privacy model for trajectory anonymization
    - The global and local mechanisms can be performed individually or composed with different ordering, which can be specified in the parameter file (detailed later).
- To support large-scale data, an efficient hierarchical grid index is provided along with the bottom-up-down search strategy.

!!! Tested with CentOS Linux (with gcc version 9.2.0) and macOS Monterey (Apple clang version 13.0.0) 

### Project structure

    CMakeLists.txt                                -- the version of cmake might be revised accordingly (currently is 3.15)
    cmake-build/                                  -- If needed, clean some files, then re-compile and re-build in your device
        config.properties                         -- the program will read parameters here
        Testing/                                  -- all testing data and some possbile outputs
          RoadNetworkInfo/NNid2lnglat.csv         -- the road network information used for simple map-matching as the preprosseing of data
          TestData-tdrive/                        -- T-Drive dataset, including 600 objects' one-week data,
                                                     note that some objects could be discarded due to few data points
          outputs/                                -- log file and the anonymized data will be outputed here
    ...                 
    main.cpp                                      -- the entry of the program
    spatial/                                      -- some basic geometry used in this program
    grids/                                        -- the basic grid defined here (not the hierarchical grid)
    io/                                           -- read/write file
    linking/                                      -- to extract trajectory signatures
    privacy/                                      -- all privacy models and relevant structures defined here (including hierarachical grid)
        local/                                    -- local mechanism implemented here
        global/                                   -- global mechanism implemented here
    ...                                           -- other paticipant classes and headers (not detailed here)

### Anonymization

To compile this project using cmake and c plus plus compiler, run this script:

    ./buildCPP.sh

If a permission denied error happens, please try to modify the permission of this script file by `chmod 700 buildCPP.sh`

To run the program compiled before, execute the command in the folder of `cmake-build`:

    ./FrequencyDP

This command will execute the program in `main.cpp`.

Datasets should be located in the `cmake-build/Testing/TestData-tdrive/` folder. To anonymize a new dataset by yourself, simply add it in this folder and modify the parameter in the file `config.properties`.

The test configuration is in `cmake-build/config.properties`. 

### Input Parameters (an example)

    RoadNetworkFile = ./Testing/RoadNetworkInfo/NNid2lnglat.csv         -- fixed, used for the simple map-matching initially
    data_FolderInput = ./Testing/TestData-tdrive/                       -- the input data to be protected
    data_FolderOutput = ./Testing/outputs/                              -- the output folder, any log file or anonymized trajectries will be here

    data_total = 100                                                    -- how many trajectories to be protected (e.g., 100, 500, etc)
    output_results = false                                              -- whether the anonymized data will be outputted
    linking_reduction = 10                                              -- for signature reduction, i.e., the number of signature points for each trajectory
    privacy_epsilon = 0.5,2.0                                           -- privacy budget for differential privacy; allow multiple choices here
    edit_global = true                                                  -- pureGlobal
    edit_local  = true                                                  -- pureLocal
    global_local_composition_order = GLLG                               -- GL: Global->Local;   LG: Local->Global;  allow both

If you encounter any issue during executing this program, please feel free to contact fengmei.jin@uq.edu.au .

Thank you!
