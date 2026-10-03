# What is it?

This is a simple include to parse GTA:SA "cargrp.dat" file. This file contains information about which vehicle models belong to a certain car group. These car groups represent singleplayer's population groups such as business workers, golfers, ballas and others or in other words, if you want to spawn a random car that belongs to a population group, then this parser will allow you to do that. https://github.com/wg1337/omp-gta-vehicles-ide

# How to use it?

1) Place "cargrp.dat" in your "scriptfiles/" folder

2) Include it in your script:
```
#include <omp_gta_cargrp>

public OnFilterScriptInit() {
    ....
    if(!LoadCarGrp()) {
        print("ERROR: Failed to load cargrp.dat");
        return false;
    }
    ....
    return true;
}
```

3) Use the provided functions, for example:
```
//Get a random vehicle modelid that would a farmer drive
new modelid = GetRandomCarGrpVehicle(GTA_CARGRP_FARMERS);
```
