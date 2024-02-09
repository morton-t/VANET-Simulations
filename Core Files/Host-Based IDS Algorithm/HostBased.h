/*
 * HostBased.h
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
#include <cmath>

// Table and trust parameters
static const int DATAPOINTS_PER_ROW = 10;
static const int SELF_TABLE_INDEX = 0;
static const int MAX_TABLE_ENTRIES = 1000;
static const int VARIANCE_THRESHOLD = 2;            //
//static const int MIN_TRUST_THRESHOLD = -5;          //
//static const int MAX_TRUST_THRESHOLD = 5;           //
//static const int TRUST_DIFFERENCE_CUTOFF = 3;       //
static const double SPEED_THRESHOLD = 5.0;          //
static const double FLOW_THRESHOLD = 100.0;

// Indexing values for Host Based table statistics rows
static const int VEH_ID_INDEX = 0;
static const int VEH_POS_INDEX = 1;
static const int VEH_SPEED_INDEX = 2;
static const int VEH_FLOW_INDEX = 3;
static const int VEH_DENSITY_INDEX = 4;
static const int VEH_TYPE_INDEX = 5;
static const int VEH_PREDICT_INDEX = 6;
static const int VEH_ENCOUNTER_TIME = 7;
static const int VEH_CLASSIFIED_TIME = 8;
static const int VEH_DETECTION_EPOCHS = 9;

// Variables for table statistics
static int numTablesInAvg = 0;
static double accuracy = 0;
static double precision = 0;
static double recall = 0;
static double specificity = 0;
static double f1Score = 0;
static double avgDetectionEpoch = 0;


class HostBased {
public:
    HostBased();
    virtual ~HostBased();

    int nextTableRow;
    int numTableEntries;
    double tableSpeedAverage;
    double tableFlowAverage;
    double tableFlowVariance;

    //~~~~~~
    int nodeDensity;
    double nodeFlow;
    double transmissionRange;
    //~~~~~~

    std::string ownerVehType; //TODO: Remove this & accessor/mutator if not needed

    std::unordered_map<std::string, int> suspectedGhosts; /** Contains vehs flagged as ghosts by trust score and a random challenge number for the vehicle */
    std::unordered_set<std::string> confirmedGhosts; /** Contains vehs confirmed as ghosts by failed challenge response */
    std::unordered_set<std::string> unreportedGhosts; /** Contains a list of ghosts which have yet to be reported to reporting authority */

    //scoreTable data entries: [vehicle ID, coordinates, speed, flow, density, vehicleType, predict, encounter time, classify time, time til classification]
    std::array<std::array<std::string, DATAPOINTS_PER_ROW>, MAX_TABLE_ENTRIES> scoreTable;

    void setTransmissionRange(double inputRange);

    std::array<std::string, DATAPOINTS_PER_ROW> getVehicleTableData(std::string vehID);

    std::array<std::array<std::string, DATAPOINTS_PER_ROW>, MAX_TABLE_ENTRIES> getHostBased();
    void setScoreTable(std::array<std::string, DATAPOINTS_PER_ROW> newEntry);
    void printScoreTable();

    double getTableFlowAverage();
    double getTableSpeedAverage();

    double getTableFlowVariance();

    void startAttackDetection(std::string incomingVehSpeed, std::string incomingVehDensity, std::string incomingVehFlow,
                        std::string incomingVehType, std::string incomingVehID, std::string incomingVehCoords, std::string encounterTime);

    void setOwnerVehType(std::string vehType);
    std::string getOwnerVehType();

    void setTableStats();
    void getTableStats();

    bool tHypothesisTest(double incomingVehFlow);

    int getNumTableEntries();
    int getNextTableRow();

    void confirmGhost(std::string vehID, std::string vehType);

 //~~~~~~~~~~~~~~~~~~~~~
    void setNodeDensity();
    int getNodeDensity();

    void setNodeFlow();
    double getNodeFlow();

    std::pair<double,double> coordToPair(std::string inputCoord);
protected:
};

#endif /* TRACI_SCORETABLE_H_ */
