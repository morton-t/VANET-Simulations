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

    for (int i = 0; i < MAX_TABLE_ENTRIES; ++i) {
        for (int j = 0; j < DATAPOINTS_PER_ROW; ++j) {
            scoreTable[i][j] = "null";
        }
    }
}

ScoreTable::~ScoreTable() {
    ////std::cout << "ScoreTable Destructed: " << this << std::endl;
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
        if (scoreTable[i][0] == newEntry[0]) {
            scoreTable[i] = newEntry;
            return;
        }
    }

    //If the vehID is not found, then create a new entry
    scoreTable[nextTableRow] = newEntry;

    if (numTableEntries < MAX_TABLE_ENTRIES) {
        numTableEntries++;
    }

    // Upkeep nextTableRow
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
        //Summation of strings converted to doubles
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

        //Calculate the sum of squared differences
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

    std::array<std::string, DATAPOINTS_PER_ROW> columnNames = {"--VehID--", "--VehSpeed--", "--Timestamp--", "--Actual--", "--Predict--", "--Trust Score--"};

    std::cout << "\t\t";
    for (int i = 0; i < columnNames.size(); ++i) {
        std::cout << std::setw(18) << columnNames[i];
    }
    std::cout << std::endl;

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

    //If the vehicle is below trust threshold, do not update its data
    if (incomingTrustScore <= MIN_TRUST_THRESHOLD) {
        return incomingVehTableData;
    }

    // BEGIN ALGORITHM 1 LOGIC
    // Check messaging vehicle's speed against speed limit
    if (std::abs(incomingVehSpeed - roadSpeedLimit) > SPEED_THRESHOLD) {
        // Check messaging vehicle's speed against table's variance
//        if (std::abs(incomingVehSpeed - getTableSpeedAverage()) > getTableSpeedVariance()) {

            // Conduct hypothesis test and decrement trust score if true
            if (tHypothesisTest(incomingVehSpeed) == true) {
                incomingTrustScore -= 1;
            }
            else if (incomingTrustScore <= MAX_TRUST_THRESHOLD) {
                incomingTrustScore += 1;
            }
//        }
    }
    // If vehicle speed is within speed limit, increment trust score
    else {
        if (incomingTrustScore <= MAX_TRUST_THRESHOLD) {
            incomingTrustScore += 1;
        }
    }

    // Check the incoming vehicle's trust score against the score threshold
    if (incomingTrustScore - getTableTrustAverage() <= TRUST_DIFFERENCE_CUTOFF) {
        // Flag and report the vehicle
        incomingVehTableData[VEH_PREDICT_INDEX] = "ghost";
        suspectedGhosts.push_back(incomingVehTableData[VEH_ID_INDEX]);
        // reportGhostVehicle(incomingVehTableData[0]); TODO: Implement reporting logic
    }

    //Update and return new table entry
    incomingVehTableData[VEH_TRUSTSCORE_INDEX] = std::to_string(incomingTrustScore);
    return incomingVehTableData;

    // END ALGORITHM 1 LOGIC



//    // BEGIN FIRST TRUST SCORE LOGIC
//    if ((incomingVehSpeed + 2.5 < std::stod(vehSpeed)) || (incomingVehSpeed - 2.5 > std::stod(vehSpeed))) {
//        //std::cout << "set " << incomingVehTableData[0] << "(" << incomingVehTableData[DATAPOINTS_PER_ROW - 3] << ")" << "'s trust score: -1" << std::endl;
//        incomingTrustScore -= 1;
//
//        // If the score falls below the trust threshold, flag the vehicle as a ghost
//        if (incomingTrustScore <= MIN_TRUST_THRESHOLD) {
//            incomingVehTableData[VEH_PREDICT_INDEX] = "ghost";
//        }
//    }
//    else {
//        //std::cout << "set " << incomingVehTableData[0] << "(" << incomingVehTableData[DATAPOINTS_PER_ROW - 3] << ")" << "'s trust score: +1" << std::endl;
//
//        //If the score is less than the trust threshold, increment the score
//        if (incomingTrustScore < MAX_TRUST_THRESHOLD) {
//            incomingTrustScore += 1;
//        }
//    }
//
//    //Update and return new table entry
//    incomingVehTableData[VEH_TRUSTSCORE_INDEX] = std::to_string(incomingTrustScore);
//    return incomingVehTableData;
//
//    // END FIRST TRUST SCORE LOGIC
}

double ScoreTable::getTableTrustAverage() {
    double average = 0.0;

    for (int i = 0; i < numTableEntries; ++i) {
        average += std::stod(scoreTable[i][VEH_TRUSTSCORE_INDEX]);
    }

    average /= numTableEntries;

    return average;
}

void ScoreTable::setTableStats() {
    /*
     * Calculates the scoretable's true/false positive/negatives
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

    // Get tp, tn, fp, fn for current table
    for (int i = 0; i < this->numTableEntries; ++i) {
        std::string vehActual = scoreTable[i][VEH_TYPE_INDEX];
        std::string vehPredict = scoreTable[i][VEH_PREDICT_INDEX];

        // Correct entries for self/attacker (both are passenger vehicles)
        if (vehActual == "attacker") {
            vehActual = "passenger";
        }
        else if (vehPredict == "self") {
            vehPredict = "passenger";
        }

        //Vehicle is a ghost
        if (vehActual == "ghost") {
            if (vehActual == vehPredict) {
                tp++;
            }
            else {
                fn++;
            }
        }
        //Vehicle is not a ghost
        else if (vehActual == "passenger") {
            if (vehActual == vehPredict) {
                tn++;
            }
            else {
                fp++;
            }
        }
        //Flag invalid table entry
        else {
            std::cout << "Invalid vehType in setTableStats()!" << std::endl;
            tp = INT_MIN;
            tn = INT_MIN;
            fp = INT_MIN;
            fn = INT_MIN;
        }
    }

    // Get stats for individual table
    if (tp + tn + fp + fn == 0) {
        tableAccuracy = 0;
    }
    else {
        tableAccuracy = (tp + tn) / (double)(tp + tn + fp + fn);
    }

    if (tp + fp == 0) {
        tablePrecision = 0;
    }
    else {
        tablePrecision = tp / (double)(tp + fp);
    }

    if (tp + fn == 0) {
        tableRecall = 0;
    }
    else {
        tableRecall = tp / (double)(tp + fn);
    }

    if (tn + fp == 0) {
        tableSpecificity = 0;
    }
    else {
        tableSpecificity = tn / (double)(tn + fp);
    }

    if (tablePrecision + tableRecall == 0.0) {
        tableF1Score = 0.0;
    }
    else {
        tableF1Score = (2 * tablePrecision * tableRecall) / (tablePrecision + tableRecall);
    }

    // Increment count of tables in running average
    numTablesInAvg++;

    // Update running average stats
    accuracy = (tempAccuracy + tableAccuracy) / numTablesInAvg;
    precision = (tempPrecision + tablePrecision) / numTablesInAvg;
    recall = (tempRecall + tableRecall) / numTablesInAvg;
    specificity = (tempSpecificity + tableSpecificity) / numTablesInAvg;
    f1Score = (tempF1Score + tableF1Score) / numTablesInAvg;
}

void ScoreTable::getTableStats() {
    std::cout << "\n" << std::endl;

    std::cout << "Accuracy: " << accuracy << std::endl;
    std::cout << "Precision: " << precision << std::endl;
    std::cout << "Recall: " << recall << std::endl;
    std::cout << "Specificity: " << specificity << std::endl;
    std::cout << "F1 Score: " << f1Score << std::endl;
}

bool ScoreTable::tHypothesisTest(double incomingVehSpeed) {
    /*
     * Conducts a two-tailed hypothesis test to determine if reported incoming speed
     * runs contradictory to the average speed reported within the score table
     */

    const double ALPHA = 0.2;
    double degreesOfFreedom = numTableEntries;
    double allTableSpeeds[numTableEntries];
    double tableSpeedVariance = getTableSpeedVariance();

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
