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
    init = true;

    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        // Initializing members and pointers of your application goes here
        EV << "Initializing " << par("appName").stringValue() << std::endl;

    }
    else if (stage == 1) {
        // Initializing members that require initialized other modules goes here
    }

    // ~~~~~~ ADD SELF TO SCORE TABLE ~~~~~~~~~~~~
    //TODO: Move to own method!

    //Pass transmission range for 802.11p to HostBased class
    st.setTransmissionRange(TRANSMISSION_RANGE);

    //Retrieve relevant self data
    Coord coords = mobility->getPositionAt(omnetpp::simTime());

    std::string vehCoords = formatCoords(coords.x, coords.y, 2);
    std::string vehSpeed = std::to_string(traciVehicle->getSpeed());
    std::string timestamp = omnetpp::simTime().str();
    vehType = traciVehicle->getVType();
    vehID = mobility->getExternalId();

    // Set table data for self
    std::array<std::string, DATAPOINTS_PER_ROW> newEntry = {vehID, vehCoords, vehSpeed, "0", "0", vehType, "self", timestamp, "null", "null"};
    st.setOwnerVehType(vehType);

    st.setScoreTable(newEntry);
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

    // NOTE: I tried to use different channels to check the message type, but the channel number is set on populating WSMs
    //      before sending, thus overriding any changes to the channel number made beforehand

    Broadcast* wsm = check_and_cast<Broadcast*>(msg);
    handleBroadcast(wsm);

    // Check the type of incoming message
//    std::string messageType = msg->getMessageType();
//
//    // Handle broadcasts with trust algorithm
//    if (messageType == "Broadcast") {
//        Broadcast* wsm = check_and_cast<Broadcast*>(msg);
//        handleBroadcast(wsm);
//    }
//    // Handle challenge message
//    else if (messageType == "Challenge") {
//        ChallengePacket* wsm = check_and_cast<ChallengePacket*>(msg);
//
//        // Check if message is intended for recipient before processing the message
//        if (wsm->getRecipient() == vehID) {
//            handleChallenge(wsm);
//        }
//    }
//    // Handle responses to challenge messages
//    else if (messageType == "ChallengeResponse") {
//        ResponsePacket* wsm = check_and_cast<ResponsePacket*>(msg);
//        // Check if message is intended for recipient before processing the message
//        if (wsm->getRecipient() == vehID) {
//            handleResponse(wsm);
//        }
//    }
//    else {
//        std::cerr << "\nUnknown message type received in onWSM()!" << std::endl;
//    }
}

void VeinsApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void VeinsApp::handleSelfMsg(cMessage* msg)
{
    /*
     * Handles messages the vehicle sends to itself for status updates
     */

    //Retrieve vehicle's own information on self message
    Coord coords = mobility->getPositionAt(omnetpp::simTime());

    std::string vehCoords = formatCoords(coords.x, coords.y, 2);
    std::string vehSpeed = std::to_string(traciVehicle->getSpeed());
    std::string vehDensity = std::to_string(st.getNodeDensity());
    std::string vehFlow = std::to_string(st.getNodeFlow());


    // Update table data & impute table's average trust score for self
    std::array<std::string, DATAPOINTS_PER_ROW> newEntry = {vehID, vehCoords, vehSpeed, vehFlow, vehDensity, vehType, "self", "0", "null", "null"};

    st.setScoreTable(newEntry);

    DemoBaseApplLayer::handleSelfMsg(msg);
}

void VeinsApp::handlePositionUpdate(cObject* obj)
{
    // The vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class

    DemoBaseApplLayer::handlePositionUpdate(obj);

    /*
     * TODO: This should prune entries from the table that are no longer within broadcast range at this point!
     */

    // NOTE: init prevents an initial message with null data from being sent before the vehicle begins movement
    if (!init) {
        // On position update, create a new message that contains the vehicle's vehID, Speed, VehType, Coordinates, and simulation timestamp

        Broadcast* wsm = new Broadcast();

        // Retrieve and set coordinate data
        Coord coords = mobility->getPositionAt(omnetpp::simTime());
        std::string vehCoords = formatCoords(coords.x, coords.y, 2);
        wsm->setVehCoords(vehCoords.c_str());

        //Set the vehicle's speed, flow, and density
        wsm->setVehSpeed(std::to_string(traciVehicle->getSpeed()).c_str());
        wsm->setVehDensity(std::to_string(st.getNodeDensity()).c_str());
        wsm->setVehFlow(std::to_string(st.getNodeFlow()).c_str());

        //Set the vehID, speed, timestamp, vehType, and coordinate information for the vehicle's outbound messages
        wsm->setVehID(vehID.c_str());
        wsm->setVehType(vehType.c_str());


        // Send wsm with above data
        populateWSM(wsm);
        sendDown(wsm);
    }
    else {
        init = !init;
    }
}

std::string VeinsApp::formatCoords(double x, double y, int precision) {
    /*
     * Formats the vehicle coordinates to make a prettier table output
     */

    std::ostringstream oss;
    oss << "(" << std::fixed << std::setprecision(precision) << x << ", " << std::setprecision(precision) << y << ")";
    return oss.str();
}

void VeinsApp::handleBroadcast(Broadcast* wsm) {
    /*
     * Processes incoming WSMs from other vehicles and implements trust table & trust score logic
     */

    // Unpack message data
    std::string incomingVehSpeed = wsm->getVehSpeed();
    std::string incomingVehDensity = wsm->getVehDensity();
    std::string incomingVehFlow = wsm->getVehFlow();
    std::string incomingVehType = wsm->getVehType();
    std::string incomingVehID = wsm->getVehID();
    std::string incomingVehCoords = wsm->getVehCoords();

//////////// BEGIN HOST-BASED IDS ALGORITHM ////////////
    st.startAttackDetection(incomingVehSpeed, incomingVehDensity, incomingVehFlow, incomingVehType, incomingVehID, incomingVehCoords, omnetpp::simTime().str());
}

