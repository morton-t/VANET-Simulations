//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "VeinsApp.h"

using namespace veins;

Define_Module(veins::VeinsApp);

void VeinsApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        // Initializing members and pointers of your application goes here
        EV << "Initializing " << par("appName").stringValue() << std::endl;

    }
    else if (stage == 1) {
        // Initializing members that require initialized other modules goes here
    }
}
void VeinsApp::finish()
{
    //NOTE: This will be called before the ScoreTable destructor so it is possible to convey
    // the table information to output in this method before the vehicle is destructed.

    std::cout << "\n\t" << mobility->getExternalId() << "'s score table:" << std::endl;
    st.printScoreTable();


    DemoBaseApplLayer::finish();
    // statistics recording goes here
}

void VeinsApp::onBSM(DemoSafetyMessage* bsm){
}

void VeinsApp::onWSM(BaseFrame1609_4* msg)
{
    // Your application has received a data message from another car or RSU

    int ROAD_SPEED_LIMIT_TEMP = 25; //FIXME: This needs to become a dynamic value; currently a temporary for proof of concept

    // Get receiver's speed
    std::string vehSpeed = std::to_string(traciVehicle->getSpeed());

    // Process the received message vehicle data
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(msg);
    std::string incomingVehID = wsm->getVehID();
    std::string incomingVehSpeed = wsm->getVehSpeed();
    std::string incomingTimeStamp = wsm->getTimestamp();
    std::string incomingVehType = wsm->getVehType();

    //ALGO LINES 3 & 4 - Retrieve incoming message's vehicle table data if present, build table entry if it doesn't exist
    std::array<std::string, DATAPOINTS_PER_ROW> incomingVehTableData = st.getVehicleTableData(incomingVehID);

    if (incomingVehTableData[0] == "null") {
        incomingVehTableData = {incomingVehID, incomingVehSpeed, incomingTimeStamp, incomingVehType, "passenger", "0", "0", "0", "0"};
    }

    //Update trust score and add to score table
    incomingVehTableData = st.setTrustScore(incomingVehTableData, vehSpeed, ROAD_SPEED_LIMIT_TEMP);
    st.setScoreTable(incomingVehTableData);


/*
    int incomingTrustScore = 0;

    std::string vehSpeed = std::to_string(traciVehicle->getSpeed());

    // Process the received message vehicle data
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(msg);
    std::string incomingVehID = wsm->getVehID();
    std::string incomingVehSpeed = wsm->getVehSpeed();
    std::string incomingTimeStamp = wsm->getTimestamp();
    std::string incomingVehType = wsm->getVehType();

    std::array<std::string, DATAPOINTS_PER_ROW> incomingVehTableData = st.getVehicleTableData(incomingVehID);

    //Check and set incoming vehicle message trust score, otherwise build table entry data minus trust score
    if (incomingVehTableData[0] != "null") {
        incomingTrustScore = std::stoi(incomingVehTableData.back());

        //If the vehicle is flagged as a ghost, do not update its score table data
        if (std::stoi(incomingVehTableData.back()) <= MIN_TRUST_THRESHOLD) { return; }
    }
    else {
        incomingVehTableData = {incomingVehID, incomingVehSpeed, incomingTimeStamp, incomingVehType, "passenger", "null"};
    }


    //BEGIN TODO: This section is not robust!! It only compares vehicles' speeds to the current speed
    // of the recipient and assigns trust score based on non-matching speeds!


    if ((std::stod(incomingVehSpeed) + 2.5 < std::stod(vehSpeed)) || (std::stod(incomingVehSpeed) - 2.5 > std::stod(vehSpeed))) {
        //std::cout << mobility->getExternalId().c_str() << "(" << traciVehicle->getVType() << ")"  <<
        //        " set " << incomingVehID << "(" << incomingVehType << ")" << "'s trust score: -1" << std::endl;
        incomingTrustScore -= 1;

        // If the score falls below the trust threshold, flag the vehicle as a ghost
        if (incomingTrustScore <= MIN_TRUST_THRESHOLD) {
            const int VEH_PREDICT_OFFSET = 2;
            incomingVehTableData[incomingVehTableData.size() - VEH_PREDICT_OFFSET] = "ghost";
        }
    }
    else {
        //std::cout << mobility->getExternalId().c_str() << "(" << traciVehicle->getVType() << ")" <<
        //        " set " << incomingVehID << "(" << incomingVehType << ")" << "'s trust score: +1" << std::endl;

        //If the score exceeds the trust threshold, do not increment the score
        if (incomingTrustScore < MAX_TRUST_THRESHOLD) {
            incomingTrustScore += 1;
        }
    }
    //END TODO

    //Update trust score and add to score table
    incomingVehTableData.back() = std::to_string(incomingTrustScore);
    st.setScoreTable(incomingVehTableData);
*/
}

void VeinsApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void VeinsApp::handleSelfMsg(cMessage* msg)
{
    //This grabs the external ID of the vehicle in the mobility module which corresponds to a specific vehicle's ID
    std::string vehID = mobility->getExternalId();

    //This retrieves the speed of the self-messaging vehicle and the simtime at which the message is received.
    std::string vehSpeed = std::to_string(traciVehicle->getSpeed());
    std::string timestamp = omnetpp::simTime().str();
    std::string vehType = traciVehicle->getVType();

    //Get self data from scoreTable if present
    std::array<std::string, DATAPOINTS_PER_ROW> selfTableData = st.getVehicleTableData(vehID);

    // Add self to score table without skewing trust score for self
    if (selfTableData[0] == "null") {
        std::array<std::string, DATAPOINTS_PER_ROW> newEntry = {vehID, vehSpeed, timestamp, vehType, "self", "0", "0", "0", "0"};
        st.setScoreTable(newEntry);
    }
    else {
        std::array<std::string, DATAPOINTS_PER_ROW> newEntry = {vehID, vehSpeed, timestamp, vehType, "self", selfTableData[VEH_TRUSTSCORE_INDEX], "0", "0", "0"};
        st.setScoreTable(newEntry);
    }

    DemoBaseApplLayer::handleSelfMsg(msg);
}

void VeinsApp::handlePositionUpdate(cObject* obj)
{
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class

    DemoBaseApplLayer::handlePositionUpdate(obj);

    // On position update, create a new message that contains the vehicle's vType and send the message to other vehicles
    //FIXME: This needs to use a custom message instead of the demo message
    TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();

    //Set the vehID, speed, timestamp, and vehType information for the vehicle's outbound messages
    wsm->setVehID(mobility->getExternalId().c_str());
    wsm->setVehSpeed(std::to_string(traciVehicle->getSpeed()).c_str());
    std::string t = omnetpp::simTime().str();
    wsm->setTimestamp(t.c_str());
    wsm->setVehType(traciVehicle->getVType().c_str());


    populateWSM(wsm);
    sendDown(wsm);
}
