#if defined _omp_gta_cargrp_included
    #endinput
#endif
#define _omp_gta_cargrp_included

#include <omp_gta_vehicles_ide>

#define CARGROUP_FILE "cargrp.dat"
#define CARGROUP_MAX_VEHICLES (23)

#define GTA_CARGRP_NAME_LENGTH (32)
#define GTA_CARGRP_MAX_LINE_LENGTH (512)

enum E_GTA_CAR_GROUP {
    GTA_CARGROUP_WORKERS = 0,
    GTA_CARGROUP_BUSINESS,
    GTA_CARGROUP_CLUBBERS,
    GTA_CARGROUP_FARMERS,
    GTA_CARGROUP_BEACHFOLK,
    GTA_CARGROUP_PARKFOLK,
    GTA_CARGROUP_CASUAL_RICH,
    GTA_CARGROUP_CASUAL_AVERAGE,
    GTA_CARGROUP_CASUAL_POOR,
    GTA_CARGROUP_PROSTITUTES,
    GTA_CARGROUP_CRIMINALS,
    GTA_CARGROUP_GOLFERS,
    GTA_CARGROUP_SERVANTS,
    GTA_CARGROUP_AIRCREW,
    GTA_CARGROUP_ENTERTAINERS,
    GTA_CARGROUP_OUT_OF_TOWN_FACTORY,
    GTA_CARGROUP_DESERT_FOLK,
    GTA_CARGROUP_AIRCREW_RUNWAY,
    GTA_CARGROUP_BALLAS,                 // 18
    GTA_CARGROUP_GROVE,                  // 19
    GTA_CARGROUP_VAGOS,                  // 20
    GTA_CARGROUP_SF_RIFA,                // 21
    GTA_CARGROUP_DA_NANG_BOYS,           // 22
    GTA_CARGROUP_ITALIAN_MAFIA,          // 23
    GTA_CARGROUP_TRIADS,                 // 24
    GTA_CARGROUP_VARRIO_LOS_AZTECAS,     // 25
    GTA_CARGROUP_UNUSED_1,                // 26
    GTA_CARGROUP_UNUSED_2,                // 27
    GTA_CARGROUP_DEALERS,                 // 28
    GTA_CARGROUP_BOATS,                   // 29
    GTA_CARGROUP_CHEAT1,                  // 30
    GTA_CARGROUP_CHEAT2,                  // 31
    GTA_CARGROUP_CHEAT3,                  // 32
    GTA_CARGROUP_CHEAT4,                  // 33

    GTA_CARGROUP_COUNT                    // dummy, uses for calculating size
};

#define CARGROUP_CIVILIAN_COUNT (_:GTA_CARGROUP_BALLAS)
#define CARGROUP_GANG_FIRST     (_:GTA_CARGROUP_BALLAS)
#define CARGROUP_GANG_COUNT     (_:GTA_CARGROUP_DEALERS - _:GTA_CARGROUP_BALLAS)
#define CARGROUP_DEALERS        (_:GTA_CARGROUP_DEALERS)
#define CARGROUP_COUNT          (_:GTA_CARGROUP_COUNT)

static gCarGroupNames[CARGROUP_COUNT][CARGROUP_MAX_VEHICLES][GTA_CARGRP_NAME_LENGTH];
static gCarGroupVehicleCount[CARGROUP_COUNT];
static gCarGroupVehicles[CARGROUP_COUNT][CARGROUP_MAX_VEHICLES];
static bool:gCarGroupsLoaded;
static bool:gCarGroupsResolved;

static stock bool:CarGroup_CopyTrimmedRange(const source[], start, end, destination[], destinationSize) {
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

static stock bool:CarGroup_IsLineEmpty(const line[]) {
    for(new i = 0; line[i] != '\0'; i++) {
        if(line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n')
            return false;
    }
    return true;
}

static stock bool:CarGroup_IsCommentLine(const line[]) {
    new i = 0;

    while(line[i] == ' ' || line[i] == '\t')
        i++;

    return line[i] == '#';
}

static stock bool:ResolveCarGroups() {
    if(!IsVehicleIdeLoaded()) {
        print("[omp-gta-cargrp] ERROR: vehicles.ide has not been loaded.");
        return false;
    }

    if(!gCarGroupsLoaded) {
        print("[omp-gta-cargrp] ERROR: cargrp.dat has not been loaded.");
        return false;
    }

    gCarGroupsResolved = false;

    for(new group = 0; group < CARGROUP_COUNT; group++) {
        for(new i = 0; i < gCarGroupVehicleCount[group]; i++) {
            new modelId = GetVehicleIdeModelIdByName(
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

static stock bool:CarGroup_ParseLine(const line[], E_GTA_CAR_GROUP:group) {
    if(group < GTA_CARGROUP_WORKERS || group >= GTA_CARGROUP_COUNT)
        return false;

    new length = strlen(line);
    new commentPos = length;

    /*
    * Ignore an optional commnets
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

            if(!CarGroup_CopyTrimmedRange(line, tokenStart, i, vehicleName, sizeof(vehicleName))) {
                printf("[omp-gta-cargrp] Empty vehicle name: group=%d", _:group);
                return false;
            }

            /*
            * CPopulation::LoadCarGroups() loads 23 groups and ignores other
            */
            if(vehicleCount >= CARGROUP_MAX_VEHICLES)
                break;

            format(gCarGroupNames[_:group][vehicleCount], GTA_CARGRP_NAME_LENGTH, "%s", vehicleName);
            vehicleCount++;
            tokenStart = i + 1;
        }
    }

    gCarGroupVehicleCount[_:group] = vehicleCount;
    return true;
}

stock bool:LoadCarGroups(const filename[] = CARGROUP_FILE) {
    new File:file = fopen(filename, io_read);

    if(!file) {
        printf("[omp-gta-cargrp] Failed to open %s", filename);
        return false;
    }

    gCarGroupsLoaded = false;
    gCarGroupsResolved = false;

    for(new group = 0; group < CARGROUP_COUNT; group++) {
        gCarGroupVehicleCount[group] = 0;

        for(new vehicle = 0; vehicle < CARGROUP_MAX_VEHICLES; vehicle++) {
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

        if(CarGroup_IsLineEmpty(line) || CarGroup_IsCommentLine(line))
            continue;

        if(groupIndex >= CARGROUP_COUNT)
            break;

        if(!CarGroup_ParseLine(line, E_GTA_CAR_GROUP:groupIndex)) {
            printf("[CarGroups] Failed to parse group %d: %s", groupIndex, line);
            fclose(file);
            return false;
        }

        groupIndex++;
    }

    fclose(file);

    if(groupIndex != CARGROUP_COUNT) {
        printf("[omp-gta-cargrp] Expected %d car groups, loaded %d.", CARGROUP_COUNT, groupIndex);
        return false;
    }

    for(new group = 0; group < CARGROUP_COUNT; group++) {
        if(gCarGroupVehicleCount[group] <= 0) {
            printf("[CarGroups] Missing/empty group: %d", group);
            return false;
        }
    }

    gCarGroupsLoaded = true;

    printf("[omp-gta-cargrp] Loaded %d car groups.", CARGROUP_COUNT);

    if(!ResolveCarGroups()) {
        gCarGroupsLoaded = false;
        return false;
    }

    return true;
}

stock bool:IsCarGroupsLoaded() {
    return gCarGroupsLoaded && gCarGroupsResolved;
}

/*
* Returns how many cars are there in a certain car group
*/
stock GetCarGroupVehicleCount(E_GTA_CAR_GROUP:group) {
    if(group < GTA_CARGROUP_WORKERS || group >= GTA_CARGROUP_COUNT)
        return 0;

    return gCarGroupVehicleCount[_:group];
}

/*
* Gets the vehicle "game_name" for a car group with offset
*/
stock bool:GetCarGroupVehicleName(E_GTA_CAR_GROUP:group, index, output[], size = sizeof(output)) {
    if(group < GTA_CARGROUP_WORKERS || group >= GTA_CARGROUP_COUNT || index < 0 || index >= gCarGroupVehicleCount[_:group]) {
        output[0] = '\0';
        return false;
    }

    format(output, size, "%s", gCarGroupNames[_:group][index]);
    return true;
}

/*
* Gets the vehicle modelid for a car group with offset
*/
stock GetCarGroupVehicleModel(E_GTA_CAR_GROUP:group, index) {
    if(!gCarGroupsResolved || group < GTA_CARGROUP_WORKERS || group >= GTA_CARGROUP_COUNT || index < 0 || index >= gCarGroupVehicleCount[_:group]) {
        return -1;
    }

    return gCarGroupVehicles[_:group][index];
}

/*
* Gets a random vehicle model from a cargrp.dat group, ignores streaming funcions
*/
stock GetRandomCarGroupVehicle(E_GTA_CAR_GROUP:group) {
    if(!gCarGroupsResolved || group < GTA_CARGROUP_WORKERS || group >= GTA_CARGROUP_COUNT) {
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
stock bool:IsVehicleInCarGroup(E_GTA_CAR_GROUP:group, modelId) {
    if(!gCarGroupsResolved || group < GTA_CARGROUP_WORKERS || group >= GTA_CARGROUP_COUNT) {
        return false;
    }

    new count = gCarGroupVehicleCount[_:group];

    for(new i = 0; i < count; i++) {
        if(gCarGroupVehicles[_:group][i] == modelId)
            return true;
    }

    return false;
}
