/*
 * ScoreTable.h
 *
 *      Author: Thomas Morton
 */

#ifndef TRACI_SCORETABLE_H_
#define TRACI_SCORETABLE_H_

#include <array>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <unordered_map>

// Table and trust parameters
static const int DATAPOINTS_PER_ROW = 11;
static const int SELF_TABLE_INDEX = 0;
static const int MAX_TABLE_ENTRIES = 1000;
static const int VARIANCE_THRESHOLD = 2;
static const int MIN_TRUST_THRESHOLD = -5;
static const int MAX_TRUST_THRESHOLD = 5;
static const int TRUST_DIFFERENCE_CUTOFF = 2;
static const double SPEED_THRESHOLD = 5.0;

// Indexing values for scoretable rows
static const int VEH_ID_INDEX = 0;
static const int VEH_POS_INDEX = 1;
static const int VEH_SPEED_INDEX = 2;
static const int VEH_TIMESTAMP_INDEX = 3;
static const int VEH_TYPE_INDEX = 4;
static const int VEH_PREDICT_INDEX = 5;
static const int VEH_TRUSTSCORE_INDEX = 6;
static const int VEH_ENCOUNTER_TIME = 7;
static const int VEH_CLASSIFIED_TIME = 8;
static const int VEH_DETECTION_EPOCHS = 9;
static const int VEH_CONFIRMED_TYPE = 10;

// Variables for table statistics
static int numTablesInAvg = 0;
static double accuracy = 0;
static double precision = 0;
static double recall = 0;
static double specificity = 0;
static double f1Score = 0;
static double avgDetectionEpoch = 0;


class ScoreTable {
public:
    ScoreTable();
    virtual ~ScoreTable();

    int nextTableRow;
    int numTableEntries;
    double tableSpeedVariance;
    double tableSpeedAverage;
    int roadSpeedLimit;

    std::string ownerVehType; //TODO: Remove this & accessor/mutator if not needed

    std::unordered_map<std::string, int> suspectedGhosts; /** Contains vehs flagged as ghosts by trust score and a random challenge number for the vehicle */
    std::unordered_set<std::string> confirmedGhosts; /** Contains vehs confirmed as ghosts by failed challenge response */
    std::unordered_set<std::string> unreportedGhosts; /** Contains a list of ghosts which have yet to be reported to reporting authority */

    //scoreTable data entries: [vehicle ID, coordinates, speed, timestamp, vehicleType, predict, trustscore]
    std::array<std::array<std::string, DATAPOINTS_PER_ROW>, MAX_TABLE_ENTRIES> scoreTable;

    std::array<std::string, DATAPOINTS_PER_ROW> getVehicleTableData(std::string vehID);

    std::array<std::array<std::string, DATAPOINTS_PER_ROW>, MAX_TABLE_ENTRIES> getScoreTable();
    void setScoreTable(std::array<std::string, DATAPOINTS_PER_ROW> newEntry);
    void printScoreTable();

    double getTableSpeedAverage();
    double getTableSpeedVariance();

    std::array<std::string, DATAPOINTS_PER_ROW> setTrustScore(std::array<std::string, DATAPOINTS_PER_ROW> incomingVehTableData, std::string vehSpeed, int roadSpeedLimit, const int TRUST_MODIFIER);
    double getTableTrustAverage();

    void setOwnerVehType(std::string vehType);
    std::string getOwnerVehType();

    void setTableStats();
    void getTableStats();

    bool tHypothesisTest(double incomingVehSpeed);

    int getNumTableEntries();
    int getNextTableRow();

    void incrementVehTrustScore(std::string vehID);
    void confirmGhost(std::string vehID, std::string vehType);

    void setRoadSpeedLimit(int limit);

protected:
};

#endif /* TRACI_SCORETABLE_H_ */
