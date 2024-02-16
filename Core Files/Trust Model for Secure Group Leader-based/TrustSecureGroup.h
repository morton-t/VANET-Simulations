/*
 * TrustSecureGroup.h
 *
 *      Author: Thomas Morton
 */

#ifndef TRACI_SCORETABLE_H_
#define TRACI_SCORETABLE_H_

#include <string>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <map>

static const double DEFAULT_TRUST_SCORE = 0.5;

//Smoothing parameters
static const double ALPHA = 0.7; /** Alpha smoothing for direct/indirect tables */
static const double BETA = 0.5; /** Beta smoothing for total tables */

// Table time management parameters (seconds)
static const int WATCHLIST_DURATION = 3; /** time on watchlist before classification as malicious */
static const double PRUNE_THRESHOLD = 2; /** duration of time to wait before pruning a table from neighboring vehicles table */

// Neighbor Table indices
static const int NEIGHBOR_TABLE_COLS = 3;
static const int VEH_POS_INDEX = 0;
static const int VEH_SPEED_INDEX = 1;
static const int VEH_TIMESTAMP_INDEX = 2;

// Total and TOTM table indices
static const int TOTAL_OBS_COLS = 6;
static const int VEH_TYPE_INDEX = 0;
static const int VEH_PREDICT_INDEX = 1;
static const int VEH_TRUSTSCORE_INDEX = 2;
static const int VEH_ENCOUNTER_TIME = 3;
static const int VEH_CLASSIFIED_TIME = 4;
static const int VEH_DETECTION_EPOCHS = 5;

// Variables for table statistics
static int numTablesInAvg = 0;
static double accuracy = 0;
static double precision = 0;
static double recall = 0;
static double specificity = 0;
static double f1Score = 0;
static double avgDetectionEpoch = 0;


class TrustSecureGroup {
public:
    TrustSecureGroup();
    virtual ~TrustSecureGroup();
    std::string vehID;

    int groupRequestsSent;
    int currentGroup;
    bool isGroupLeader;
    bool isBackupGroupLeader;
    static int groupID;

    std::unordered_map<std::string, std::array<std::string, NEIGHBOR_TABLE_COLS>> neighboringVehicles; /** maintains a list of neighbors and timestamp of last encounter */
    std::unordered_map<int, std::pair<std::string, std::string>> groupLeaders; /** Key: groupID, Value: pair with (groupLeader, backupGroupLeader) */

    // Observation tables
    std::unordered_map<std::string, double> directObservationTable; /** Maintains the direct observation trust scores for a given vehID */
    std::unordered_map<std::string, double> indirectObservationTable; /** Maintains the indirect observation trust scores for a given vehID */
    std::unordered_map<std::string, std::array<std::string, TOTAL_OBS_COLS>> totalObservationTable; /** Maintains the total observation scores trust for a given vehID */
    std::map<std::string, double> globalObservationTable; /** Maintains the global observation scores trust for a given vehID */
    std::unordered_map<std::string, double> watchList; /** Maintains a list of vehicles exhibiting suspicious activity */

    void joinGroup(int group, std::string groupLeader, std::string backupGroupLeader);

    // Neighbor table maintenance methods
    void pruneNeighborTable(double timestamp);
    void updateNeighborVehicles(std::string *vehID, std::string *vehCoords, double *speed, std::string *timestamp);

    // Table updating methods
    void updateDirectObservationTable(std::string *vehicle, double *trustScore);
    void updateIndirectObservationTable(const std::string *vehicle, const double *trustScore);
    void updateTotalObservationTable(std::string *vehicle, std::string *vehType, std::string *timestamp);
    void updateTotMObservationTable();

    std::string getHighestTotalTrust();

    // Coordinate processing methods
    std::pair<double,double> coordToPair(std::string inputCoord);
    double getEuclideanDistance(std::string vehCoords, std::string incomingVehCoords);

    // 802.11p processing methods
    void handleBroadcast(std::string *vehID, std::string *vehCoords, double *speed, std::string *timestamp, std::string *vehType, int *roadSpeedLimit, std::unordered_map<int, std::pair<std::string, std::string>> *incomingGroupLeaders);
    void handleTrustList(std::unordered_map<std::string, double> *trustList);

    double getNeighborSpeedAvg();
    double getTrustThreshold();

    void printTrustSecureGroup();

    void setTableStats();
    void getTableStats();

protected:
};

#endif /* TRACI_SCORETABLE_H_ */
