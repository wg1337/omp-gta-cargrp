#if defined _omp_gta_cargrp_included
    #endinput
#endif
#define _omp_gta_cargrp_included

#include <omp_gta_vehicles_ide>

#define GTA_CARGRP_FILE "cargrp.dat"

/* In GTA:SA only 23 values are read for each cargrp.dat entry, but some entries have more than 23 cars listed. If you want more vehicles in some groups, you should increase this number */
#define GTA_CARGRP_MAX_VEHICLES (23)

#define GTA_CARGRP_NAME_LENGTH (32)
#define GTA_CARGRP_MAX_LINE_LENGTH (512)
#define GTA_CARGRP_MAX_COUNT (34)

enum E_GTA_CARGRP {
    GTA_CARGRP_WORKERS = 0,
    GTA_CARGRP_BUSINESS,
    GTA_CARGRP_CLUBBERS,
    GTA_CARGRP_FARMERS,
    GTA_CARGRP_BEACHFOLK,
    GTA_CARGRP_PARKFOLK,
    GTA_CARGRP_CASUAL_RICH, /* because of 23 limit, skipping euros, phoenix, emperor */
    GTA_CARGRP_CASUAL_AVERAGE, /* becuase of 23 limit, skipping tahoma, picador, bmx, tornado, euros, sentinel, vcsenXS */
    GTA_CARGRP_CASUAL_POOR,
    GTA_CARGRP_PROSTITUTES,
    GTA_CARGRP_CRIMINALS,
    GTA_CARGRP_GOLFERS,
    GTA_CARGRP_SERVANTS,
    GTA_CARGRP_AIRCREW,
    GTA_CARGRP_ENTERTAINERS,
    GTA_CARGRP_OUT_OF_TOWN_FACTORY,
    GTA_CARGRP_DESERT_FOLK,
    GTA_CARGRP_AIRCREW_RUNWAY,
    GTA_CARGRP_BALLAS,                 // 18
    GTA_CARGRP_GROVE,                  // 19
    GTA_CARGRP_VAGOS,                  // 20
    GTA_CARGRP_SF_RIFA,                // 21
    GTA_CARGRP_DA_NANG_BOYS,           // 22
    GTA_CARGRP_ITALIAN_MAFIA,          // 23
    GTA_CARGRP_TRIADS,                 // 24
    GTA_CARGRP_VARRIO_LOS_AZTECAS,     // 25
    GTA_CARGRP_UNUSED_1,                // 26
    GTA_CARGRP_UNUSED_2,                // 27
    GTA_CARGRP_DEALERS,                 // 28
    GTA_CARGRP_BOATS,                   // 29
    GTA_CARGRP_CHEAT1,                  // 30
    GTA_CARGRP_CHEAT2,                  // 31
    GTA_CARGRP_CHEAT3,                  // 32
    GTA_CARGRP_CHEAT4                  // 33
};

#define GTA_CARGRP_CIVILIAN_COUNT (_:GTA_CARGRP_BALLAS)
#define GTA_CARGRP_GANG_FIRST (_:GTA_CARGRP_BALLAS)
#define GTA_CARGRP_GANG_COUNT (_:GTA_CARGRP_DEALERS - _:GTA_CARGRP_BALLAS)


static gCarGroupNames[GTA_CARGRP_MAX_COUNT][GTA_CARGRP_MAX_VEHICLES][GTA_CARGRP_NAME_LENGTH];
static gCarGroupVehicleCount[GTA_CARGRP_MAX_COUNT];
static gCarGroupVehicles[GTA_CARGRP_MAX_COUNT][GTA_CARGRP_MAX_VEHICLES];
static bool:gCarGroupsLoaded;
static bool:gCarGroupsResolved;

static stock bool:CarGrp_CopyTrimmedRange(const source[], start, end, destination[], destinationSize) {
    while(start < end && (source[start] == ' ' || source[start] == '\t'))
        start++;

    while(end > start && (source[end - 1] == ' ' || source[end - 1] == '\t' || source[end - 1] == '\r' || source[end - 1] == '\n'))
        end--;

    new length = end - start;

    if(length <= 0 || length >= destinationSize)
        return false;

    for(new i = 0; i < length; i++)
        destination[i] = source[start + i];

    destination[length] = '\0';
    return true;
}

static stock bool:CarGrp_IsLineEmpty(const line[]) {
    for(new i = 0; line[i] != '\0'; i++) {
        if(line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n')
            return false;
    }
    return true;
}

static stock bool:CarGrp_IsCommentLine(const line[]) {
    new i = 0;

    while(line[i] == ' ' || line[i] == '\t')
        i++;

    return line[i] == '#';
}

static stock bool:ResolveCarGrp() {
    if(!IsVehiclesIdeLoaded()) {
        print("[omp-gta-cargrp] ERROR: vehicles.ide has not been loaded.");
        return false;
    }

    if(!gCarGroupsLoaded) {
        print("[omp-gta-cargrp] ERROR: cargrp.dat has not been loaded.");
        return false;
    }

    gCarGroupsResolved = false;

    for(new group = 0; group < GTA_CARGRP_MAX_COUNT; group++) {
        for(new i = 0; i < gCarGroupVehicleCount[group]; i++) {
            new modelId = GetVehiclesIdeModelIdByName(
                gCarGroupNames[group][i]
            );

            if(modelId == -1) {
                printf("[omp-gta-cargrp] ERROR: Unable to resolve '%s'", gCarGroupNames[group][i]);
                return false;
            }

            gCarGroupVehicles[group][i] = modelId;
        }
    }

    gCarGroupsResolved = true;
    print("[omp-gta-cargrp] Resolved all vehicle references.");
    return true;
}

static stock bool:CarGrp_ParseLine(const line[], E_GTA_CARGRP:group) {
    if(group < GTA_CARGRP_WORKERS || group >= E_GTA_CARGRP:GTA_CARGRP_MAX_COUNT)
        return false;

    new length = strlen(line);
    new commentPos = length;

    /*
    * Ignore an optional comments
    */
    for(new i = 0; i < length; i++) {
        if(line[i] == '#') {
            commentPos = i;
            break;
        }
    }

    new first = 0;
    while(first < commentPos && (line[first] == ' ' || line[first] == '\t'))
        first++;

    if(first >= commentPos)
        return false;

    new vehicleCount = 0;
    new tokenStart = first;

    for(new i = first; i <= commentPos; i++) {
        if(i == commentPos || line[i] == ',') {
            new vehicleName[GTA_CARGRP_NAME_LENGTH];

            if(!CarGrp_CopyTrimmedRange(line, tokenStart, i, vehicleName, sizeof(vehicleName))) {
                printf("[omp-gta-cargrp] Empty vehicle name: group=%d", _:group);
                return false;
            }

            /*
            * CPopulation::LoadCarGroups() loads 23 cars and ignores other
            */
            if(vehicleCount >= GTA_CARGRP_MAX_VEHICLES)
                break;

            format(gCarGroupNames[_:group][vehicleCount], GTA_CARGRP_NAME_LENGTH, "%s", vehicleName);
            vehicleCount++;
            tokenStart = i + 1;
        }
    }

    gCarGroupVehicleCount[_:group] = vehicleCount;
    return true;
}

stock bool:LoadCarGrp(const filename[] = GTA_CARGRP_FILE) {
    new File:file = fopen(filename, io_read);

    if(!file) {
        printf("[omp-gta-cargrp] Failed to open %s", filename);
        return false;
    }

    gCarGroupsLoaded = false;
    gCarGroupsResolved = false;

    for(new group = 0; group < GTA_CARGRP_MAX_COUNT; group++) {
        gCarGroupVehicleCount[group] = 0;

        for(new vehicle = 0; vehicle < GTA_CARGRP_MAX_VEHICLES; vehicle++) {
            gCarGroupNames[group][vehicle][0] = '\0';
            gCarGroupVehicles[group][vehicle] = -1;
        }
    }

    new line[GTA_CARGRP_MAX_LINE_LENGTH];
    new groupIndex = 0;

    while(fread(file, line)) {
        new length = strlen(line);

        while(length > 0 && (line[length - 1] == '\r' || line[length - 1] == '\n'))
            line[--length] = '\0';

        if(CarGrp_IsLineEmpty(line) || CarGrp_IsCommentLine(line))
            continue;

        if(groupIndex >= GTA_CARGRP_MAX_COUNT)
            break;

        if(!CarGrp_ParseLine(line, E_GTA_CARGRP:groupIndex)) {
            printf("[omp-gta-cargrp] Failed to parse group %d: %s", groupIndex, line);
            fclose(file);
            return false;
        }

        groupIndex++;
    }

    fclose(file);

    if(groupIndex != GTA_CARGRP_MAX_COUNT) {
        printf("[omp-gta-cargrp] Expected %d car groups, loaded %d.", GTA_CARGRP_MAX_COUNT, groupIndex);
        return false;
    }

    for(new group = 0; group < GTA_CARGRP_MAX_COUNT; group++) {
        if(gCarGroupVehicleCount[group] <= 0) {
            printf("[omp-gta-cargrp] Missing/empty group: %d", group);
            return false;
        }
    }

    gCarGroupsLoaded = true;

    printf("[omp-gta-cargrp] Loaded %d car groups.", GTA_CARGRP_MAX_COUNT);

    if(!ResolveCarGrp()) {
        gCarGroupsLoaded = false;
        return false;
    }

    return true;
}

stock bool:IsCarGrpLoaded() {
    return gCarGroupsLoaded && gCarGroupsResolved;
}

/*
* Returns how many cars are there in a certain car group
*/
stock GetCarGrpVehicleCount(E_GTA_CARGRP:group) {
    if(group < GTA_CARGRP_WORKERS || group >= E_GTA_CARGRP:GTA_CARGRP_MAX_COUNT)
        return 0;

    return gCarGroupVehicleCount[_:group];
}

/*
* Gets the vehicle "model name" for a car group with offset
*/
stock bool:GetCarGrpVehicleName(E_GTA_CARGRP:group, index, output[], size = sizeof(output)) {
    if(group < GTA_CARGRP_WORKERS || group >= E_GTA_CARGRP:GTA_CARGRP_MAX_COUNT || index < 0 || index >= gCarGroupVehicleCount[_:group]) {
        output[0] = '\0';
        return false;
    }

    format(output, size, "%s", gCarGroupNames[_:group][index]);
    return true;
}

/*
* Gets the vehicle modelid for a car group with offset
*/
stock GetCarGrpVehicleModel(E_GTA_CARGRP:group, index) {
    if(!gCarGroupsResolved || group < GTA_CARGRP_WORKERS || group >= E_GTA_CARGRP:GTA_CARGRP_MAX_COUNT || index < 0 || index >= gCarGroupVehicleCount[_:group]) {
        return -1;
    }

    return gCarGroupVehicles[_:group][index];
}

/*
* Gets a random vehicle model from a cargrp.dat group, ignores streaming funcions
*/
stock GetRandomCarGrpVehicle(E_GTA_CARGRP:group) {
    if(!gCarGroupsResolved || group < GTA_CARGRP_WORKERS || group >= E_GTA_CARGRP:GTA_CARGRP_MAX_COUNT) {
        return -1;
    }

    new count = gCarGroupVehicleCount[_:group];

    if(count <= 0)
        return -1;

    return gCarGroupVehicles[_:group][random(count)];
}

/*
* Checks if a modelid belongs to a car group
*/
stock bool:IsVehicleInCarGrp(E_GTA_CARGRP:group, modelId) {
    if(!gCarGroupsResolved || group < GTA_CARGRP_WORKERS || group >= E_GTA_CARGRP:GTA_CARGRP_MAX_COUNT) {
        return false;
    }

    new count = gCarGroupVehicleCount[_:group];

    for(new i = 0; i < count; i++) {
        if(gCarGroupVehicles[_:group][i] == modelId)
            return true;
    }

    return false;
}
