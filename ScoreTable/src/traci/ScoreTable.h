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

// Table and trust parameters
static const int MAX_TABLE_ENTRIES = 1000;
static const int DATAPOINTS_PER_ROW = 6;
static const int VARIANCE_THRESHOLD = 2;
static const int MIN_TRUST_THRESHOLD = -2;
static const int MAX_TRUST_THRESHOLD = 2;
static const int TRUST_DIFFERENCE_CUTOFF = -1;
static const double SPEED_THRESHOLD = 5.0;

// Indexing values for scoretable rows
static const int VEH_ID_INDEX = 0;
static const int VEH_SPEED_INDEX = 1;
static const int VEH_TIMESTAMP_INDEX = 2;
static const int VEH_TYPE_INDEX = 3;
static const int VEH_PREDICT_INDEX = 4;
static const int VEH_TRUSTSCORE_INDEX = 5;

// Variables for table statistics
static int numTablesInAvg = 0;
static double accuracy = 0;
static double precision = 0;
static double recall = 0;
static double specificity = 0;
static double f1Score = 0;


class ScoreTable {
public:
    ScoreTable();
    virtual ~ScoreTable();

    int nextTableRow;
    int numTableEntries;
    double tableSpeedVariance;
    double tableSpeedAverage;

    std::vector<std::string> suspectedGhosts;

    //   NOTE:
    //   scoreTable data entries: [vehicle ID, speed, timestamp, vehicleType, predict, trustscore]
    //   vehType and verdict are not used in implementation - only used for assessment of accuracy, etc.
    std::array<std::array<std::string, DATAPOINTS_PER_ROW>, MAX_TABLE_ENTRIES> scoreTable;

    std::array<std::string, DATAPOINTS_PER_ROW> getVehicleTableData(std::string vehID);

    std::array<std::array<std::string, DATAPOINTS_PER_ROW>, MAX_TABLE_ENTRIES> getScoreTable();
    void setScoreTable(std::array<std::string, DATAPOINTS_PER_ROW> newEntry);
    void printScoreTable();

    double getTableSpeedAverage();
    double getTableSpeedVariance();

    std::array<std::string, DATAPOINTS_PER_ROW> setTrustScore(std::array<std::string, DATAPOINTS_PER_ROW> incomingVehTableData, std::string vehSpeed, int roadSpeedLimit, const int TRUST_MODIFIER);
    double getTableTrustAverage();

    void setTableStats();
    void getTableStats();

    bool tHypothesisTest(double incomingVehSpeed);

protected:
};

#endif /* TRACI_SCORETABLE_H_ */
