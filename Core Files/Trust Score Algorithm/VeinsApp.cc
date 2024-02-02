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

    //Retrieve relevant self data
    Coord coords = mobility->getPositionAt(omnetpp::simTime());

    std::string vehCoords = formatCoords(coords.x, coords.y);
    std::string vehSpeed = std::to_string(traciVehicle->getSpeed());
    std::string timestamp = omnetpp::simTime().str();
    vehType = traciVehicle->getVType();
    vehID = mobility->getExternalId();

    // Set table data for self
    std::array<std::string, DATAPOINTS_PER_ROW> newEntry = {vehID, vehCoords, vehSpeed, timestamp, vehType, "self", "0", "0", "null", "null", "null"};
    st.setOwnerVehType(newEntry[VEH_TYPE_INDEX]);

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


    // Check the type of incoming message
    std::string messageType = msg->getMessageType();

    // Handle broadcasts with trust algorithm
    if (messageType == "Broadcast") {
        Broadcast* wsm = check_and_cast<Broadcast*>(msg);
        handleBroadcast(wsm);
    }
    // Handle challenge message
    else if (messageType == "Challenge") {
        ChallengePacket* wsm = check_and_cast<ChallengePacket*>(msg);

        // Check if message is intended for recipient before processing the message
        if (wsm->getRecipient() == vehID) {
            handleChallenge(wsm);
        }
    }
    // Handle responses to challenge messages
    else if (messageType == "ChallengeResponse") {
        ResponsePacket* wsm = check_and_cast<ResponsePacket*>(msg);
        // Check if message is intended for recipient before processing the message
        if (wsm->getRecipient() == vehID) {
            handleResponse(wsm);
        }
    }
    else {
        std::cerr << "\nUnknown message type received in onWSM()!" << std::endl;
    }
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

    std::string vehCoords = formatCoords(coords.x, coords.y);
    std::string vehID = mobility->getExternalId();
    std::string vehSpeed = std::to_string(traciVehicle->getSpeed());
    std::string timestamp = omnetpp::simTime().str();


    // Update table data & impute table's average trust score for self
    std::array<std::string, DATAPOINTS_PER_ROW> newEntry = {vehID, vehCoords, vehSpeed, timestamp, vehType, "self", std::to_string((int)round(st.getTableTrustAverage())), "0", "null", "null", "null"};
    st.setScoreTable(newEntry);

    DemoBaseApplLayer::handleSelfMsg(msg);
}

void VeinsApp::handlePositionUpdate(cObject* obj)
{
    // The vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class

    DemoBaseApplLayer::handlePositionUpdate(obj);

    // NOTE: init prevents an initial message with null data from being sent before the vehicle begins movement
    if (!init) {
        // On position update, create a new message that contains the vehicle's vehID, Speed, VehType, Coordinates, and simulation timestamp
        Broadcast* wsm = new Broadcast();
        wsm->setMessageType("Broadcast");

        // Retrieve coordinate data
        Coord coords = mobility->getPositionAt(omnetpp::simTime());
        std::string vehCoords = formatCoords(coords.x, coords.y);

        //Set the vehID, speed, timestamp, vehType, and coordinate information for the vehicle's outbound messages
        wsm->setVehID(mobility->getExternalId().c_str());
        wsm->setVehSpeed(std::to_string(traciVehicle->getSpeed()).c_str());
        wsm->setTimestamp(omnetpp::simTime().str().c_str());
        wsm->setVehType(traciVehicle->getVType().c_str());
        wsm->setVehCoords(vehCoords.c_str());

        // Send wsm with above data
        populateWSM(wsm);
        sendDown(wsm);
    }
    else {
        init = !init;
    }
}

std::string VeinsApp::formatCoords(double x, double y) {
    /*
     * Formats the vehicle coordinates to make a prettier table output
     */

    std::ostringstream oss;
    oss << "(" << std::fixed << std::setprecision(2) << x << ", " << std::setprecision(2) << y << ")";
    return oss.str();
}

void VeinsApp::handleBroadcast(Broadcast* wsm) {
    /*
     * Processes incoming WSMs from other vehicles and implements trust table & trust score logic
     */

    int ROAD_SPEED_LIMIT_TEMP = 25; //FIXME: This needs to become a dynamic value; currently a temporary for proof of concept

    // Get receiver's speed
    std::string vehSpeed = std::to_string(traciVehicle->getSpeed());

    // Process the received message vehicle data
    std::string incomingVehID = wsm->getVehID();
    std::string incomingVehSpeed = wsm->getVehSpeed();
    std::string incomingTimeStamp = wsm->getTimestamp();
    std::string incomingVehType = wsm->getVehType();
    std::string incomingVehCoords = wsm->getVehCoords();

    int trustModifier = 0;

    //ALGO LINES 3 & 4 - Retrieve incoming message's vehicle table data if present, build table entry if it doesn't exist
    std::array<std::string, DATAPOINTS_PER_ROW> incomingVehTableData = st.getVehicleTableData(incomingVehID);

    // Create new table entry
    if (incomingVehTableData[0] == "null") {
        incomingVehTableData = {incomingVehID, incomingVehCoords, incomingVehSpeed, incomingTimeStamp, incomingVehType, "passenger", std::to_string(0 + trustModifier), incomingTimeStamp, "null", "null", "null"};
    }
    // Update dynamically reported values for existing vehicle entry
    else {
        incomingVehTableData[VEH_SPEED_INDEX] = incomingVehSpeed;
        incomingVehTableData[VEH_TIMESTAMP_INDEX] = incomingTimeStamp;
        incomingVehTableData[VEH_POS_INDEX] = incomingVehCoords;
    }

    //Update trust score and add to score table
    incomingVehTableData = st.setTrustScore(incomingVehTableData, vehSpeed, ROAD_SPEED_LIMIT_TEMP, trustModifier);
    st.setScoreTable(incomingVehTableData);

    // Iterate through suspected ghosts and create challenge packets for all vehicles present
    for (auto& vehicle : st.suspectedGhosts) {
        createChallenge(vehicle.first);
    }
}

void VeinsApp::handleChallenge(ChallengePacket* wsm) {
    /*
     * Processes incoming challenge packets from other vehicles with message type "Challenge"
     */

    /*
     * Ghost vehicles will be unable to answer challenge packets sent from a directional antenna, since
     * there will not be a vehicle at the given coordinate to receive the packet. Thus, we return an
     * invalid number "-1" to symbolize a failure to receive the message.
     */

    /*
     * TODO: Implement back-off scheme for failure to receive messages and neglect responses from ghosts
     *       Ideally, a vehicle should be reported as a ghost if it fails to respond to N number of messages rather than the current approach
     */


    // Generate and send a response packet to the received challenge packet
    ResponsePacket* response = new ResponsePacket();

    // Populate response packet data
    response->setMessageType("ChallengeResponse");
    response->setSender(vehID.c_str());
    response->setRecipient(wsm->getSender());

    // If the recipient is a ghost, set response to -1
    if (vehType == "ghost") {
        return;                                     //TODO: For some reason, not sending a response from a ghost results in better precision
                                                    //      Need to investigate the cause of the anomaly
        // response->setChallengeResponse(-1);
    }
    // If the recipient is not a ghost, we answer the challenge correctly
    else {
        response->setChallengeResponse(wsm->getChallengeNumber());
    }

    // Send response packet
    populateWSM(response);
    sendDown(response);
}

void VeinsApp::handleResponse(ResponsePacket* wsm) {
    /*
     * Processes incoming response packets from other vehicles with message type "ChallengeResponse"
     */

    std::string sender = wsm->getSender();
    int senderResponse = wsm->getChallengeResponse();

    // Check if suspected ghost responded with the correct challenge number
    if (st.suspectedGhosts[sender] == senderResponse) {
        // Response is correct - increment the trust score for the suspected ghost
        st.incrementVehTrustScore(sender);
        st.confirmGhost(sender, "passenger");
    }
    else {
        // Response is incorrect - vehicle is confirmed as a ghost

        // Add vehicle to reporting unordered sets
        st.confirmedGhosts.insert(sender);
        st.unreportedGhosts.insert(sender);
        st.confirmGhost(sender, "ghost");

        //TODO: This is not a secure approach to validation! Need to implement public key exchange in practice.
    }
}

void VeinsApp::createChallenge(std::string recipient) {
    /*
     * Generates a challenge packet with a random number to send to a suspect ghost vehicle.
     * If a previously generated random number exists for the vehicle, reuses the previous number
     */

    // Zero is used a control number and is invalid for random number generation
    int rand = 0;

    // Check if the vehicle is in suspectedGhosts and reuse the previously sent random number if it is.
    if (st.suspectedGhosts.find(recipient) == st.suspectedGhosts.end()) {
        //Generate a non-zero random number
        while (rand == 0) {
            rand = std::rand();
        }
    }
    else {
        rand = st.suspectedGhosts[recipient];
    }

    //Generate challenge packet
    ChallengePacket* wsm = new ChallengePacket();

    // Set message data
    wsm->setMessageType("Challenge");
    wsm->setSender(vehID.c_str());
    wsm->setRecipient(recipient.c_str());

    // Assign a random number for a challenge number
    wsm->setChallengeNumber(rand);

    // Update suspected ghosts hash table with the last sent random number for the given vehicle
    st.suspectedGhosts.insert({recipient, rand});

    // Send wsm with above data
    populateWSM(wsm);
    sendDown(wsm);
}
