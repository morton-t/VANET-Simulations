/*
 * ScoreTable.cpp
 *
 *      Author: Thomas Morton
 */

#include "ScoreTable.h"
#include "omnetpp.h"

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>


ScoreTable::ScoreTable() : nextTableRow(0), numTableEntries(0), tableSpeedVariance(0.0) {
    ////std::cout << "ScoreTable Constructed: " << this << std::endl;

    //initialize scoreTable with null values
    for (int i = 0; i < MAX_TABLE_ENTRIES; ++i) {
        for (int j = 0; j < DATAPOINTS_PER_ROW; ++j) {
            scoreTable[i][j] = "null";
        }
    }
}

ScoreTable::~ScoreTable() {
    ////std::cout << "ScoreTable Destructed: " << this << std::endl;

    //Update and retrieve statistics data for the vehicle's score table
    setTableStats();
    getTableStats();
}

std::array<std::string, DATAPOINTS_PER_ROW> ScoreTable::getVehicleTableData(std::string vehID) {
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

std::array<std::array<std::string, DATAPOINTS_PER_ROW>, MAX_TABLE_ENTRIES>  ScoreTable::getScoreTable() {
    return scoreTable;
}

void ScoreTable::setScoreTable(std::array<std::string, DATAPOINTS_PER_ROW> newEntry) {
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

double ScoreTable::getTableSpeedAverage() {
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


double ScoreTable::getTableSpeedVariance() {
    /*
     * Calculates the variance of the score table's vehicle speeds and returns the result
     */

    if (numTableEntries >= VARIANCE_THRESHOLD) {
        double tableSpeedAverage = getTableSpeedAverage();
        double sumSqDiff = 0.0;

        for (int i = 0; i < numTableEntries; ++i) {
            sumSqDiff += std::pow(std::stod(scoreTable[i][VEH_SPEED_INDEX]) - tableSpeedAverage, 2);
        }

        tableSpeedVariance = sumSqDiff / numTableEntries;
    }

    return tableSpeedVariance;
}

void ScoreTable::printScoreTable() {
    /*
     * Prints the score table entries under their given column names
     */

    std::array<std::string, DATAPOINTS_PER_ROW> columnNames = {"--VehID--", "--Coordinates--", "--VehSpeed--", "--Timestamp--", "--Actual--", "--Predict--", "--Trust Score--", "--Encountered--", "--Detected--", "--Epochs--", "--Confirmed--"};

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

std::array<std::string, DATAPOINTS_PER_ROW> ScoreTable::setTrustScore(std::array<std::string, DATAPOINTS_PER_ROW> incomingVehTableData, std::string vehSpeed, int roadSpeedLimit, const int TRUST_MODIFIER) {
    /*
     * Generates trust score values for the received vehicle ID based on its reported msg data
     */

    double incomingVehSpeed = std::stod(incomingVehTableData[VEH_SPEED_INDEX]);
    int incomingTrustScore = std::stoi(incomingVehTableData[VEH_TRUSTSCORE_INDEX]) + TRUST_MODIFIER;

    //If the vehicle is below trust threshold, do attempt to update its trust information
//    if (incomingTrustScore <= MIN_TRUST_THRESHOLD) {
//        return incomingVehTableData;
//    }

    // BEGIN ALGORITHM 1 LOGIC

    // If we have confirmed the vehicle as non-malicious, set its trust score to the table average  if it was less trusted than avg
    if (incomingVehTableData[VEH_CONFIRMED_TYPE] == "passenger") {
        int avgTrust = (int)round(getTableTrustAverage());
        if (incomingTrustScore < avgTrust){
            incomingTrustScore = avgTrust;
        }
        //incomingTrustScore = MAX_TRUST_THRESHOLD;
    }
    else if (incomingVehTableData[VEH_CONFIRMED_TYPE] == "ghost") {
        incomingVehTableData[VEH_PREDICT_INDEX] = "ghost";
    }
    // Poll to update trust information by check messaging vehicle's speed against speed threshold
    if (incomingVehSpeed > roadSpeedLimit * 1.2 || incomingVehSpeed < roadSpeedLimit * .8/*std::abs(incomingVehSpeed - getTableSpeedAverage()) > roadSpeedLimit * 0.2*/) {  //THIS IS THE BEST RESULT WITH: min = -5, max = 5, thresh = 5
        // Conduct hypothesis test and decrement trust score if true
        if (tHypothesisTest(incomingVehSpeed) == true && incomingTrustScore > MIN_TRUST_THRESHOLD) {
            incomingTrustScore -= 1;
        }
        else if (incomingTrustScore < MAX_TRUST_THRESHOLD) {
            //incomingTrustScore += 1;
        }
    }
    // If vehicle speed is within average speed, increment trust score
    else {
        if (incomingTrustScore < MAX_TRUST_THRESHOLD) {
            incomingTrustScore += 1;
        }
    }

    // Check the incoming vehicle's trust score against the other trust scores in the table
    if (getTableTrustAverage() - incomingTrustScore >= TRUST_DIFFERENCE_CUTOFF) {
        // Flag vehicle as ghost and update classification timestamps
        if (incomingVehTableData[VEH_CONFIRMED_TYPE] != "passenger") {
            incomingVehTableData[VEH_PREDICT_INDEX] = "ghost";
            if (incomingVehTableData[VEH_CLASSIFIED_TIME] == "null") {
                incomingVehTableData[VEH_CLASSIFIED_TIME] = incomingVehTableData[VEH_TIMESTAMP_INDEX];
                incomingVehTableData[VEH_DETECTION_EPOCHS] = std::to_string(std::stoi(incomingVehTableData[VEH_CLASSIFIED_TIME]) - std::stoi(incomingVehTableData[VEH_ENCOUNTER_TIME]));

            }
        }
        // Upkeep unordered map with suspected vehicle and a default control challenge packet value
        suspectedGhosts[incomingVehTableData[VEH_ID_INDEX]] = 0;
    }

    //Update and return new table entry
    incomingVehTableData[VEH_TRUSTSCORE_INDEX] = std::to_string(incomingTrustScore);
    return incomingVehTableData;

    // END ALGORITHM 1 LOGIC
}

void ScoreTable::setOwnerVehType(std::string vehType) {
    ownerVehType = vehType;
}

std::string ScoreTable::getOwnerVehType() {
    return ownerVehType;
}

double ScoreTable::getTableTrustAverage() {
    /*
     * Calculates and returns an average of the trust scores in the score table
     */

    double average = 0.0;

    for (int i = 0; i < numTableEntries; ++i) {
        average += std::stod(scoreTable[i][VEH_TRUSTSCORE_INDEX]);
    }

    average /= numTableEntries;

    return average;
}

void ScoreTable::setTableStats() {
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

void ScoreTable::getTableStats() {
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

bool ScoreTable::tHypothesisTest(double incomingVehSpeed) {
    /*
     * Conducts a two-tailed hypothesis test to determine if reported incoming speed
     * runs contradictory to the average speed reported within the score table
     */

    const double ALPHA = 0.05;
    double degreesOfFreedom = numTableEntries;
    double allTableSpeeds[numTableEntries];

    if (numTableEntries < 1) return false;  // No entries in table for testing

    for (int i = 0; i < numTableEntries; ++i) {
        allTableSpeeds[i] = std::stod(scoreTable[i][VEH_SPEED_INDEX]);
    }

    double t = (getTableSpeedAverage() - incomingVehSpeed) * std::sqrt(numTableEntries) / getTableSpeedVariance();

    double criticalValue = gsl_cdf_tdist_Pinv(ALPHA / 2, degreesOfFreedom);

    if (t >= criticalValue || t <= -criticalValue) {
        return true;
    }

    return false;
}

int ScoreTable::getNumTableEntries() {
    return numTableEntries;
}

int ScoreTable::getNextTableRow() {
    return nextTableRow;
}

void ScoreTable::incrementVehTrustScore(std::string vehID) {
    /*
     * Increments the trust score value in table for the given VehID and unflags veh as ghost if criteria met
     */

    for (int i = 0; i < numTableEntries; ++i) {
        // Locate vehicle in table
        if (scoreTable[i][VEH_ID_INDEX] == vehID) {
            int trustScore = std::stoi(scoreTable[i][VEH_TRUSTSCORE_INDEX]);

            //Increment trust score and update table
            if (trustScore < MAX_TRUST_THRESHOLD){
                trustScore++;
                scoreTable[i][VEH_TRUSTSCORE_INDEX] = std::to_string(trustScore);
            }

            // Veh was confirmed as a passenger by directional message
            if (scoreTable[i][VEH_CONFIRMED_TYPE] == "passenger") {
                scoreTable[i][VEH_PREDICT_INDEX] = "passenger";
                scoreTable[i][VEH_CLASSIFIED_TIME] = "null";
                scoreTable[i][VEH_DETECTION_EPOCHS] = "null";
            }
            // Veh not confirmed as passenger by message, but veh exhibits trustworthy behavior
            else if (scoreTable[i][VEH_PREDICT_INDEX] == "ghost" && (getTableTrustAverage() - trustScore) < TRUST_DIFFERENCE_CUTOFF) {
                scoreTable[i][VEH_PREDICT_INDEX] = "passenger";
                scoreTable[i][VEH_CLASSIFIED_TIME] = "null";
                scoreTable[i][VEH_DETECTION_EPOCHS] = "null";
            }

            return;
        }
    }

    std::cerr << "Vehicle ID " << vehID << " not located in trust table for " << scoreTable[0][VEH_ID_INDEX] << " on call to incrementVehTrustScore()!" << std::endl;
}

void ScoreTable::confirmGhost(std::string vehID, std::string vehType) {
    for (int i = 0; i < numTableEntries; ++i) {
        if (scoreTable[i][VEH_ID_INDEX] == vehID) {
            scoreTable[i][VEH_PREDICT_INDEX] = vehType;
            scoreTable[i][VEH_CONFIRMED_TYPE] = vehType;
            return;
        }
    }
}

void ScoreTable::setRoadSpeedLimit(int limit) {
    roadSpeedLimit = limit;
}
