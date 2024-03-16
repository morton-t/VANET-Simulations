/*
 * HostBased.cpp
 *
 *      Author: Thomas Morton
 */

#include "HostBased.h"
#include "omnetpp.h"

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>


HostBased::HostBased() : nextTableRow(0), numTableEntries(0), tableFlowVariance(0.0) {
    ////std::cout << "HostBased Constructed: " << this << std::endl;

    //initialize scoreTable with null values
    for (int i = 0; i < MAX_TABLE_ENTRIES; ++i) {
        for (int j = 0; j < DATAPOINTS_PER_ROW; ++j) {
            scoreTable[i][j] = "null";
        }
    }
}

HostBased::~HostBased() {
    ////std::cout << "HostBased Destructed: " << this << std::endl;

    //Update and retrieve statistics data for the vehicle's score table
    setTableStats();
    getTableStats();
}

void HostBased::setTransmissionRange(double input) {
    transmissionRange = input;
}

std::array<std::string, DATAPOINTS_PER_ROW> HostBased::getVehicleTableData(std::string vehID) {
    //Return score vehicle data from table
    for (int i = 0; i < numTableEntries; ++i) {
        if (scoreTable[i][0] == vehID) {
            return scoreTable[i];
        }
    }

    // If the data is not in the score table, return an array of null
    std::array<std::string, DATAPOINTS_PER_ROW> temp;

    for (int i = 0; i < DATAPOINTS_PER_ROW; ++i) {
        temp[i] = "null";
    }
    return temp;
}

std::array<std::array<std::string, DATAPOINTS_PER_ROW>, MAX_TABLE_ENTRIES>  HostBased::getHostBased() {
    return scoreTable;
}

void HostBased::setScoreTable(std::array<std::string, DATAPOINTS_PER_ROW> newEntry) {
    //TODO: This is better implemented using a hash table instead of an array

    //Search for vehID in scoreTable & update values if found
    for (int i = 0; i < this->numTableEntries; ++i) {
        if (scoreTable[i][VEH_ID_INDEX] == newEntry[VEH_ID_INDEX]) {
            scoreTable[i] = newEntry;
            return;
        }
    }

    //If the vehID is not found, then create a new entry
    scoreTable[nextTableRow] = newEntry;

    // Increment the number of entries in the score table if it is less than the max allowed entries
    if (numTableEntries < MAX_TABLE_ENTRIES) {
        numTableEntries++;
    }

    // Upkeep nextTableRow by rolling index
    if (nextTableRow + 1 == MAX_TABLE_ENTRIES) {
        nextTableRow = 0;
    }
    else {
        nextTableRow++;
    }
}

double HostBased::getTableFlowAverage() {
    /*
     * Calculates the average of the score table's vehicle speeds and returns the result
     */

    double flowSum = 0.0;

    for (int i = 0; i < numTableEntries; ++i) {
        flowSum += std::stod(scoreTable[i][VEH_FLOW_INDEX]);
    }
    tableFlowAverage = flowSum / numTableEntries;

    return tableFlowAverage;
}

double HostBased::getTableSpeedAverage() {
    /*
     * Calculates the average of the score table's vehicle speeds and returns the result
     */

    double speedSum = 0.0;

    for (int i = 0; i < numTableEntries; ++i) {
        speedSum += std::stod(scoreTable[i][VEH_SPEED_INDEX]);
    }
    tableSpeedAverage = speedSum / numTableEntries;

    return tableSpeedAverage;
}


double HostBased::getTableFlowVariance() {
    /*
     * Calculates the variance of the score table's vehicle speeds and returns the result
     */

    if (numTableEntries >= VARIANCE_THRESHOLD) {
        double tableFlowAverage = getTableFlowAverage();
        double sumSqDiff = 0.0;

        for (int i = 0; i < numTableEntries; ++i) {
            sumSqDiff += std::pow(std::stod(scoreTable[i][VEH_FLOW_INDEX]) - tableFlowAverage, 2);
        }

        tableFlowVariance = sumSqDiff / numTableEntries;
    }

    return tableFlowVariance;
}

void HostBased::printScoreTable() {
    /*
     * Prints the score table entries under their given column names
     */

    std::array<std::string, DATAPOINTS_PER_ROW> columnNames = {"--VehID--", "--Coordinates--", "--VehSpeed--", "--Flow--", "--Density--", "--Actual--", "--Predict--", "--Encountered--", "--Classified--", "--Epochs--"};

    // Print column headers
    std::cout << "\t\t";
    for (int i = 0; i < columnNames.size(); ++i) {
        std::cout << std::setw(18) << columnNames[i];
    }
    std::cout << std::endl;

    // Print remainder of table
    for (int i = 0; i < numTableEntries; ++i) {
        std::cout << "\t\t";
        for (int j = 0; j < DATAPOINTS_PER_ROW; ++j) {
            std::cout << std::setw(18) << scoreTable[i][j];
        }
        std::cout << std::endl;
    }
    std::setw(0);
}

void HostBased::startAttackDetection(std::string incomingVehSpeed, std::string incomingVehDensity, std::string incomingVehFlow,
                                    std::string incomingVehType, std::string incomingVehID, std::string incomingVehCoords, std::string encounterTime) {
    /*
     * Generates trust score values for the received vehicle ID based on its reported msg data
     */

    //////////// BEGIN HOST-BASED IDS ALGORITHM ////////////

    // Fetch existing table data (if any)
    std::array<std::string, DATAPOINTS_PER_ROW> oldEntry = getVehicleTableData(incomingVehID);
    std::array<std::string, DATAPOINTS_PER_ROW> newEntry;

    // For each message received - update density and average table speed & calculate flow -- methods' accessor updates values on call; calling getNodeFlow() calls getTableSpeedAverage() & getNodeDensity()
    getNodeFlow();

    /****/
    // We calculate the flow threshold as within 1 std dev of the observed average flow? None given in article, this is an assumption..
    double flow_th = sqrt(getTableFlowVariance());
    /****/


    // Check flow from message vs incoming flow calculation
    //if (std::stod(incomingVehFlow) - nodeFlow < FLOW_THRESHOLD) {      //FIXME: No logical value for flow_threshold implemented. Using 100.0 for the moment
    if (std::stod(incomingVehFlow) - nodeFlow < flow_th) {
        // Accept - update table data

        // If table data does not exist, create a new table entry
        if (oldEntry[VEH_ID_INDEX] == "null") {
            newEntry = {incomingVehID, incomingVehCoords, incomingVehSpeed, incomingVehFlow, incomingVehDensity, incomingVehType, "passenger", encounterTime, "null", "null"};
            setScoreTable(newEntry);
        }
        // Otherwise, maintain old data
        else {
            newEntry = oldEntry;

            // Previous data is still valid - update data reported by incoming message
            newEntry[VEH_POS_INDEX] = incomingVehCoords;
            newEntry[VEH_SPEED_INDEX] = incomingVehSpeed;
            newEntry[VEH_FLOW_INDEX] = incomingVehFlow;
            newEntry[VEH_DENSITY_INDEX] = incomingVehDensity;

            // Accept vehicle
            setScoreTable(newEntry);

            // Calculate new flow average
            getTableFlowAverage();
        }
    }
    // Vehicle data is suspicious - hold data temporarily for t-test
    else {
        // If table data does not exist, create a new table entry
        if (oldEntry[VEH_ID_INDEX] == "null") {
            newEntry = {incomingVehID, incomingVehCoords, incomingVehSpeed, incomingVehFlow, incomingVehDensity, incomingVehType, "passenger", encounterTime, "null", "null"};
        }
        // Otherwise, maintain old data
        else {
            newEntry = oldEntry;

            // Previous data is still valid - update data reported by incoming message
            newEntry[VEH_POS_INDEX] = incomingVehCoords;
            newEntry[VEH_SPEED_INDEX] = incomingVehSpeed;
            newEntry[VEH_FLOW_INDEX] = incomingVehFlow;
            newEntry[VEH_DENSITY_INDEX] = incomingVehDensity;
        }

        // Conduct t-test
        if (tHypothesisTest(std::stod(incomingVehFlow)) == true) {
            // Hypothesis rejects - data rejected and vehicle reported
            newEntry[VEH_PREDICT_INDEX] = "ghost";
            newEntry[VEH_CLASSIFIED_TIME] = encounterTime;
            newEntry[VEH_DETECTION_EPOCHS] = std::to_string(std::stod(encounterTime) - std::stod(newEntry[VEH_ENCOUNTER_TIME]));

            //reportGhost();

            // Flag entry in table
            setScoreTable(newEntry);

            /// Calculate new flow average
            getTableFlowAverage();
        }
    }
}

void HostBased::setOwnerVehType(std::string vehType) {
    ownerVehType = vehType;
}

std::string HostBased::getOwnerVehType() {
    return ownerVehType;
}

void HostBased::setTableStats() {
    /*
     * Calculates the score table's true/false positive/negatives
     * and updates a running average of these stats for all tables in the simulation
     * This method is called during scoretable's destructor call, thus increments tables in avg at start of call
     */

    // Maintains stats for current table
    double tableAccuracy = 0;
    double tablePrecision = 0;
    double tableRecall = 0;
    double tableSpecificity = 0;
    double tableF1Score = 0;

    // Maintains predictions for current table
    int tp = 0;
    int fp = 0;
    int tn = 0;
    int fn = 0;

    // Intermediate values to keep a running average for all tables
    double tempAccuracy = accuracy * numTablesInAvg;
    double tempPrecision = precision * numTablesInAvg;
    double tempRecall = recall * numTablesInAvg;
    double tempSpecificity = specificity * numTablesInAvg;
    double tempF1Score = f1Score * numTablesInAvg;
    double tempDetectionEpoch = avgDetectionEpoch * numTablesInAvg;

    // Get tp, tn, fp, fn for current table
    for (int i = 0; i < numTableEntries; ++i) {
        std::string vehActual = scoreTable[i][VEH_TYPE_INDEX];
        std::string vehPredict = scoreTable[i][VEH_PREDICT_INDEX];

        // Correct entries for self/attacker
        if (vehActual == "attacker") {
            vehActual = "passenger"; // An attacker is always a physical passenger vehicle
        }
        if (vehPredict == "self") {
            vehPredict = vehActual; // A vehicle can always correctly identify itself
        }

        //Vehicle is a ghost
        if (vehActual == "ghost") {
            if (vehPredict == "ghost") {
                tp++;
            }
            else {
                fn++;
            }
        }
        //Vehicle is not a ghost
        else if (vehActual == "passenger") {
            if (vehPredict == "passenger") {
                tn++;
            }
            else {
                fp++;
            }
        }
        //Flag invalid table entry
        else {
            std::cerr << "Invalid vehType: " << vehActual << "  in setTableStats()!" << std::endl;
            tp = INT_MIN;
            tn = INT_MIN;
            fp = INT_MIN;
            fn = INT_MIN;
        }
    }


    // Accuracy
    if (tp + tn + fp + fn == 0) {
        tableAccuracy = accuracy;
    }
    else {
        tableAccuracy = (tp + tn) / (double)(tp + tn + fp + fn);
    }

    // Precision
    if (tp + fp == 0) {
        tablePrecision = precision; // No data - impute previous value into average
    }
    else {
        tablePrecision = tp / (double)(tp + fp);
    }

    // Recall
    if (tp + fn == 0) {
        tableRecall = recall; // No data - impute previous value into average
    }
    else {
        tableRecall = tp / (double)(tp + fn);
    }

    // Specificity
    if (tn + fp == 0) {
        tableSpecificity = specificity; // No data - impute previous value into average
    }
    else {
        tableSpecificity = tn / (double)(tn + fp);
    }

    // F1 Score
    if (tablePrecision + tableRecall == 0.0) {
        tableF1Score = f1Score; // No data - impute previous value into average
    }
    else {
        tableF1Score = (2 * tablePrecision * tableRecall) / (tablePrecision + tableRecall);
    }

    // Get detection epochs for individual table
    double vehAvgDetectionEpoch = 0.0;
    int numEpochInstances = 0;

    for (int i = 0; i < numTableEntries; ++i) {
        if (scoreTable[i][VEH_DETECTION_EPOCHS] != "null") {
            numEpochInstances += 1;
            vehAvgDetectionEpoch += std::stod(scoreTable[i][VEH_DETECTION_EPOCHS]);
        }
    }
    // Calculate avg for table or set avg to overall avg if result is null
    if (vehAvgDetectionEpoch > 0.0) {
        vehAvgDetectionEpoch /= numEpochInstances;
    }
    else {
        vehAvgDetectionEpoch = avgDetectionEpoch; // No data - impute previous value into average
    }


    // Increment count of tables in running average
    numTablesInAvg++;

    // Update running average stats
    accuracy = (tempAccuracy + tableAccuracy) / numTablesInAvg;
    precision = (tempPrecision + tablePrecision) / numTablesInAvg;
    recall = (tempRecall + tableRecall) / numTablesInAvg;
    specificity = (tempSpecificity + tableSpecificity) / numTablesInAvg;
    f1Score = (tempF1Score + tableF1Score) / numTablesInAvg;
    avgDetectionEpoch = (tempDetectionEpoch + vehAvgDetectionEpoch) / numTablesInAvg;
}

void HostBased::getTableStats() {
    /*
     * Prints the current recorded table statistics
     */

    std::cout << "\n" << std::endl;

    std::cout << "Accuracy: " << accuracy << std::endl;
    std::cout << "Precision: " << precision << std::endl;
    std::cout << "Recall: " << recall << std::endl;
    std::cout << "Specificity: " << specificity << std::endl;
    std::cout << "F1 Score: " << f1Score << std::endl;
    std::cout << "Average Detection Time: " << avgDetectionEpoch << std::endl;
}

bool HostBased::tHypothesisTest(double incomingVehFlow) {
    /*
     * Conducts a two-tailed hypothesis test to determine if reported incoming flow
     * runs contradictory to the average flow reported within the score table
     */

    const double ALPHA = 0.01;
    double degreesOfFreedom = numTableEntries;
    double allTableFlows[numTableEntries];

    if (numTableEntries < 1) return false;  // No entries in table for testing

    for (int i = 0; i < numTableEntries; ++i) {
        allTableFlows[i] = std::stod(scoreTable[i][VEH_FLOW_INDEX]);
    }

    double t = (getTableFlowAverage() - incomingVehFlow) * std::sqrt(numTableEntries) / getTableFlowVariance();

    double criticalValue = gsl_cdf_tdist_Pinv(ALPHA / 2, degreesOfFreedom);

    if (t >= criticalValue || t <= -criticalValue) {
        return true;
    }

    return false;
}

int HostBased::getNumTableEntries() {
    return numTableEntries;
}

int HostBased::getNextTableRow() {
    return nextTableRow;
}


//void HostBased::confirmGhost(std::string vehID, std::string vehType) {
//    for (int i = 0; i < numTableEntries; ++i) {
//        if (scoreTable[i][VEH_ID_INDEX] == vehID) {
//            scoreTable[i][VEH_CONFIRMED_TYPE] = vehType;
//            return;
//        }
//    }
//}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HostBased::setNodeDensity() {
    nodeDensity = 1; // There will always be at least the passenger vehicle on the road

    // Convert table coordinates back to pair
    std::pair<double,double> selfCoords = coordToPair(scoreTable[0][VEH_POS_INDEX]);
    std::pair<double,double> tableCoords;

    // Calculate Euclidean distance distance of entries in table and increment node density
    // by 1 for each vehicle within the transmission range
    for (int i = 1; i < numTableEntries; ++i) {
        tableCoords = coordToPair(scoreTable[i][VEH_POS_INDEX]);

        if (std::sqrt( (std::pow(tableCoords.first - selfCoords.first, 2)) + (std::pow(tableCoords.second - selfCoords.second, 2)) ) < transmissionRange) {
                nodeDensity++;
        }
    }
}

int HostBased::getNodeDensity() {
    setNodeDensity(); //Update density

    return nodeDensity;
}

void HostBased::setNodeFlow() {
    nodeFlow = (getTableSpeedAverage() * getNodeDensity());
}

double HostBased::getNodeFlow() {
    setNodeFlow(); //Update flow calculation

    return nodeFlow;
}

std::pair<double,double> HostBased::coordToPair(std::string inputCoord) {
    std::pair<double,double> coords;
    // Remove parentheses from coordinate string
    inputCoord = inputCoord.substr(1, inputCoord.size() - 2);

    // Split X and Y coords by substring
    coords.first = std::stod(inputCoord.substr(0, inputCoord.find(",")));
    coords.second = std::stod(inputCoord.substr(inputCoord.find(",") + 1));

    return coords;
}
