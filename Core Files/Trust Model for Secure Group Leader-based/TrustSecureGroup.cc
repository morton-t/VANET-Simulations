/*
 * TrustSecureGroup.cpp
 *
 *      Author: Thomas Morton
 */

#include "TrustSecureGroup.h"
#include "omnetpp.h"

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>

//Static var maintains unique group IDs upon increment
int TrustSecureGroup::groupID = 0;

TrustSecureGroup::TrustSecureGroup() : groupRequestsSent(3), currentGroup(-1), isGroupLeader(false), isBackupGroupLeader(false) {
    ////std::cout << "TrustSecureGroup Constructed: " << this << std::endl;

    for (auto& entry : neighboringVehicles) {
        // Fill the neighboringVehicles array with "null" values
        std::fill(entry.second.begin(), entry.second.end(), "null");
    }
}

TrustSecureGroup::~TrustSecureGroup() {
    ////std::cout << "TrustSecureGroup Destructed: " << this << std::endl;

    //Update and retrieve statistics data for the vehicle's score table
//    setTableStats();
//    getTableStats();
}

void TrustSecureGroup::joinGroup(int group, std::string groupLeader, std::string backupGroupLeader) {
    /*
     * Checks if the host vehicle is part of a broadcast group - joins group if so and creates group if not
     */

    // If not GL and not in a group, join the broadcasting group
    if (currentGroup == -1) {
        if (group != -1) {
            currentGroup = group;
            groupLeaders[group] = std::make_pair(groupLeader, backupGroupLeader);

        }
        // If no eligible broadcasting group, create a group and assign a BGL
        else {
            // Create group
            currentGroup = groupID;

//            std::cout << "New group created. ID: " << groupID << std::endl;

            // Assign bgl; bgl = "null" if no other vehicles observed
            if (backupGroupLeader == "null") {
                std::string bgl = getHighestTotalTrust();
                groupLeaders[groupID] = std::make_pair(groupLeader, bgl);
            }
            else {
                groupLeaders[groupID] = std::make_pair(groupLeader, backupGroupLeader);
            }
            ++groupID;
        }
    }
}

void TrustSecureGroup::pruneNeighborTable(double timestamp) {
    /*
     * Iterates through neighboringVehicles table and removes expired entries
     */
//return; //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    for (auto it = neighboringVehicles.begin(); it != neighboringVehicles.end();) {
        if (timestamp > (std::stod(it->second[VEH_TIMESTAMP_INDEX]) + PRUNE_THRESHOLD)) {

            // Purge missing BGL from group leaders by electing a new BGL
            if (isGroupLeader && (groupLeaders[currentGroup].second == it->first)) {
                groupLeaders[currentGroup].second = getHighestTotalTrust();
            }
            // Purge missing GL from group and report to BGL instead -- GLs should not purge themselves
            else if (isBackupGroupLeader && groupLeaders[currentGroup].first == it->first) {
                groupLeaders[currentGroup].first = groupLeaders[currentGroup].second;
                groupLeaders[currentGroup].second = getHighestTotalTrust();

                // If bgl, change flags
                if (isBackupGroupLeader) {
                    isBackupGroupLeader = false;
                    isGroupLeader = true;
                }
            }

            it = neighboringVehicles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void TrustSecureGroup::updateNeighborVehicles(std::string *vehID, std::string *vehCoords, double *speed, std::string *timestamp) {
    /*
     * Updates the vehicle's list of known neighbors and data transmitted as of last communication time
     */
    neighboringVehicles[*vehID] = {*vehCoords, std::to_string(*speed), *timestamp};
}


void TrustSecureGroup::updateDirectObservationTable(std::string *vehicle, double *trustScore) {
    /*
     * Maintains a table of direct observation trust scores for a given vehicle
     */

    // Create an entry with the default trust value if one doesn't exist
    if (directObservationTable.find(*vehicle) == directObservationTable.end()) {
        directObservationTable[*vehicle] = DEFAULT_TRUST_SCORE;
    }

    // Apply smoothing to newly observed values
    directObservationTable[*vehicle] = ALPHA * (*trustScore) + ((1 - ALPHA) * directObservationTable[*vehicle]);
}

void TrustSecureGroup::updateIndirectObservationTable(const std::string *vehicle, const double *trustScore) {
    /*
     * Maintains a table of indirect observation trust scores for a given vehicle
     */

    // Create an entry with the default trust value if one doesn't exist
    if (indirectObservationTable.find(*vehicle) == indirectObservationTable.end()) {
        indirectObservationTable[*vehicle] = DEFAULT_TRUST_SCORE;
    }

    // Apply smoothing to newly observed values
    indirectObservationTable[*vehicle] = ALPHA * (*trustScore) + ((1 - ALPHA) * indirectObservationTable[*vehicle]);
}

void TrustSecureGroup::updateTotalObservationTable(std::string *vehicle, std::string *vehType, std::string *timestamp) {
    /*
     * Maintains a table of combined direct and indirect observation trust scores for a given vehicle
     */

    // Ensure an indirect observation exists, set default trust if not
    if (indirectObservationTable.find(*vehicle) == indirectObservationTable.end()) {
        indirectObservationTable[*vehicle] = DEFAULT_TRUST_SCORE;
    }

    // Check if an observation table entry exists for the given vehicle -- create one if none exists
    if (totalObservationTable.find(*vehicle) == totalObservationTable.end()) {
        for (int i = 0; i < TOTAL_OBS_COLS; ++i) {
            totalObservationTable[*vehicle][i] = "null";
        }
    }

    // Set first encounter timestamp
    if (totalObservationTable[*vehicle][VEH_ENCOUNTER_TIME] == "null") {
        totalObservationTable[*vehicle][VEH_ENCOUNTER_TIME] = *timestamp;
    }

    //Set actual vehicle type parameter
    if (totalObservationTable[*vehicle][VEH_TYPE_INDEX] == "null") {
        totalObservationTable[*vehicle][VEH_TYPE_INDEX] = *vehType;
    }

    // Set initial prediction to "passenger"
    if (totalObservationTable[*vehicle][VEH_PREDICT_INDEX] == "null") {
        totalObservationTable[*vehicle][VEH_PREDICT_INDEX] = "passenger";
    }

    // Calculate and set total trust score for vehicle
    double totalScore = BETA * directObservationTable[*vehicle] + ((1 - BETA) * indirectObservationTable[*vehicle]);
    totalObservationTable[*vehicle][VEH_TRUSTSCORE_INDEX] = std::to_string(totalScore);


    // Attempt classification of vehicle
    double tThreshold = getTrustThreshold();
    double av_i = directObservationTable[*vehicle] / indirectObservationTable[*vehicle]; //accordance parameter

    // Observation matches reports from neighbors
    if (av_i > 0.999 && av_i < 1.001) {
        // Check if vehicle passes threshold for honesty
        if (directObservationTable[*vehicle] > tThreshold) {
            totalObservationTable[*vehicle][VEH_PREDICT_INDEX] = "passenger";
            totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME] = "null";
            totalObservationTable[*vehicle][VEH_DETECTION_EPOCHS] = "null";
        }
        else {
            totalObservationTable[*vehicle][VEH_PREDICT_INDEX] = "ghost";
            totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME] = *timestamp;
            totalObservationTable[*vehicle][VEH_DETECTION_EPOCHS] = std::to_string(std::stod(totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME]) - std::stod(totalObservationTable[*vehicle][VEH_ENCOUNTER_TIME]));
        }
    }
    // Observations more trusted than reports from neighbors
    else if (av_i > 1) {
        // Check if vehicle passes threshold for honesty
        if (directObservationTable[*vehicle] > tThreshold) {
            totalObservationTable[*vehicle][VEH_PREDICT_INDEX] = "passenger";
            totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME] = "null";
            totalObservationTable[*vehicle][VEH_DETECTION_EPOCHS] = "null";
        }
        // If not, vehicle is suspicious -- investigate and then flag if honesty doesn't improve
        else {
            // If vehicle is being watched, check if it exceeded time to display honesty & flag as malicious if so
            if (watchList[*vehicle] && (watchList[*vehicle] - std::stod(*timestamp) > WATCHLIST_DURATION)) {
                totalObservationTable[*vehicle][VEH_PREDICT_INDEX] = "ghost";
                totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME] = *timestamp;
                totalObservationTable[*vehicle][VEH_DETECTION_EPOCHS] = std::to_string(std::stod(totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME]) - std::stod(totalObservationTable[*vehicle][VEH_ENCOUNTER_TIME]));
            }
            else {
                watchList[*vehicle] = std::stod(*timestamp);
            }
        }
    }
    // Observations are worse than reports from neighbors
    else if (av_i < 1) {
        // Check if vehicle passes threshold for honesty -- add to watchList
        if (directObservationTable[*vehicle] > tThreshold) {
            if (watchList[*vehicle] && (watchList[*vehicle] - std::stod(*timestamp) > WATCHLIST_DURATION)) {
                totalObservationTable[*vehicle][VEH_PREDICT_INDEX] = "ghost";
                totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME] = *timestamp;
                totalObservationTable[*vehicle][VEH_DETECTION_EPOCHS] = std::to_string(std::stod(totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME]) - std::stod(totalObservationTable[*vehicle][VEH_ENCOUNTER_TIME]));
            }
            else {
                watchList[*vehicle] = std::stod(*timestamp);
            }
        }
        // If not honest, flag as malicious
        else {
            totalObservationTable[*vehicle][VEH_PREDICT_INDEX] = "ghost";
            totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME] = *timestamp;
            totalObservationTable[*vehicle][VEH_DETECTION_EPOCHS] = std::to_string(std::stod(totalObservationTable[*vehicle][VEH_CLASSIFIED_TIME]) - std::stod(totalObservationTable[*vehicle][VEH_ENCOUNTER_TIME]));
        }
    }
}

void TrustSecureGroup::updateTotMObservationTable() {
    //TODO: not implemented

    std::cerr << "updateTotMObservationTable() not implemented " << std::endl;

}

std::string TrustSecureGroup::getHighestTotalTrust() {
    /*
     * Returns the vehID of the most trusted vehicle in the host's total observation table
     */

    std::string key = "null";

    if (totalObservationTable.empty()) {
        return key;
    }
    else {
        double highest = -1;

        for (auto it = totalObservationTable.begin(); it != totalObservationTable.end(); ++it) {
            if (std::stod(it->second[VEH_TRUSTSCORE_INDEX]) > highest) {
                highest = std::stod(it->second[VEH_TRUSTSCORE_INDEX]);
                key = it->first;
            }
        }
    }
    return key;
}

std::pair<double,double> TrustSecureGroup::coordToPair(std::string inputCoord) {
    /*
     * Converts coordinates passed as a string to a pair
     */

    std::pair<double,double> coords;

    // Remove parentheses from coordinate string
    inputCoord = inputCoord.substr(1, inputCoord.size() - 2);

    // Split X and Y coords by substring
    coords.first = std::stod(inputCoord.substr(0, inputCoord.find(",")));
    coords.second = std::stod(inputCoord.substr(inputCoord.find(",") + 1));

    return coords;
}

double TrustSecureGroup::getEuclideanDistance(std::string vehCoords, std::string incomingVehCoords) {
    /*
     * Calculates the Euclidean distance between two sets of coordinates
     */

    // Convert table coordinates back to pair
    std::pair<double,double> selfCoords = coordToPair(vehCoords);
    std::pair<double,double> incomingCoords = coordToPair(incomingVehCoords);

    // Calculate Euclidean distance
    return std::sqrt( (std::pow(incomingCoords.first - selfCoords.first, 2)) + (std::pow(incomingCoords.second - selfCoords.second, 2)) );
}

void TrustSecureGroup::handleBroadcast(std::string *vehID, std::string *vehCoords, double *speed, std::string *timestamp, std::string *vehType, int *roadSpeedLimit, std::unordered_map<int, std::pair<std::string, std::string>> *incomingGroupLeaders) {
    /*
     * Processes data received from standard BSMs between vehicles and updates the direct and total observation tables
     */

    // Add broadcasted vehicle to neighbor list
    updateNeighborVehicles(vehID, vehCoords, speed, timestamp);

    // Normalize and calculate trust based on speed values
    double trustMetricMk;

    if (*speed > *roadSpeedLimit * 1.25 ||  *speed < *roadSpeedLimit * .75) {
        trustMetricMk = 0.1;
    }
    else if (*speed >= *roadSpeedLimit * .75 && *speed < *roadSpeedLimit * .85 || *speed > *roadSpeedLimit * 1.15 && *speed <= *roadSpeedLimit * 1.25) {
        trustMetricMk = 0.5;
    }
    else if (*speed >= *roadSpeedLimit * .85 && *speed < *roadSpeedLimit * .9 || *speed > *roadSpeedLimit * 1.10 && *speed <= *roadSpeedLimit * 1.15) {
        trustMetricMk = 0.7;
    }
    else if (*speed >= *roadSpeedLimit * .9 &&  *speed <= *roadSpeedLimit * 1.1) {
        trustMetricMk = 0.9;
    }

    // Update direct trust observations
    updateDirectObservationTable(vehID, &trustMetricMk);

    // Update total trust observations
    updateTotalObservationTable(vehID, vehType, timestamp);
}

void TrustSecureGroup::handleTrustList(std::unordered_map<std::string, double> *trustList) {
    /*
     * Process incoming broadcasts of other vehicles' direct observations into indirect observation table
     */

    for (auto it = trustList->begin(); it != trustList->end(); ++it) {
        updateIndirectObservationTable(&(it->first), &(it->second));
    }
}

double TrustSecureGroup::getNeighborSpeedAvg() {
    double avg = 0;
    int n = neighboringVehicles.size();

    for (auto it = neighboringVehicles.begin(); it != neighboringVehicles.end(); ++it) {
        avg += std::stod(it->second[VEH_SPEED_INDEX]);
    }

    return avg / n;
}

double TrustSecureGroup::getTrustThreshold() {
    /*
     * Calculates trust threshold t_threshold from the vehicle's total observation table
     */

    double avg = 0;
    int n = totalObservationTable.size();

    for (auto it = totalObservationTable.begin(); it != totalObservationTable.end(); ++it) {
        avg += std::stod(it->second[VEH_TRUSTSCORE_INDEX]);
    }
    return avg / n;
}

void TrustSecureGroup::printTrustSecureGroup() {
    /*
     * Prints the total observation table entries under their given column names
     */

    std::array<std::string, TOTAL_OBS_COLS + 1> columnNames = {"--VehID--",  "--Actual--", "--Predict--", "--Trust Score--", "--Encountered--", "--Classified--", "--Epochs--"};

    // Print column headers
    std::cout << "\t\t";
    for (int i = 0; i < columnNames.size(); ++i) {
        std::cout << std::setw(18) << columnNames[i];
    }
    std::cout << std::endl;

    // Print remainder of table
    for (auto it = totalObservationTable.begin(); it != totalObservationTable.end(); ++it) {
        std::cout << "\t\t" << std::setw(18) << it->first;
        for (int i = 0; i < it->second.size(); ++i) {
            std::cout << std::setw(18) << it->second[i];
        }
        std::cout << std::endl;
    }
    std::setw(0);
}

void TrustSecureGroup::setTableStats() {
    /*
     * Calculates the score table's true/false positive/negatives
     * and updates a running average of these stats for all tables in the simulation
     * This method is called during the vehicle's destructor call, thus increments tables in avg at start of call
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
    for (auto it = totalObservationTable.begin(); it != totalObservationTable.end(); ++it) {
        std::string vehActual = it->second[VEH_TYPE_INDEX];
        std::string vehPredict = it->second[VEH_PREDICT_INDEX];

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

    for (auto it = totalObservationTable.begin(); it != totalObservationTable.end(); ++it) {
        if (it->second[VEH_DETECTION_EPOCHS] != "null") {
            numEpochInstances += 1;
            vehAvgDetectionEpoch += std::stod(it->second[VEH_DETECTION_EPOCHS]);
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

void TrustSecureGroup::getTableStats() {
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
