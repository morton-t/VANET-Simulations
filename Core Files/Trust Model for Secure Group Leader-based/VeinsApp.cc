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

    roadSpeedLimit = 35;        // All simulations 35 as the maximum road speed -- this can be update to be dynamic

    //Retrieve relevant self data
    Coord coords = mobility->getPositionAt(omnetpp::simTime());

    vehID = mobility->getExternalId();
    vehType = traciVehicle->getVType();
    vehSpeed = traciVehicle->getSpeed();
    timestamp = std::stod(omnetpp::simTime().str());
    vehCoords = formatCoords(coords.x, coords.y, 2);

    initTime = timestamp;

    // Initialize first vehicle to group 0 and as the group leader
    if (!firstLeaderSet) {
        tsg.joinGroup(-1, vehID, "null");
        tsg.isGroupLeader = true;
        firstLeaderSet = true;

        std::cout << "(First Group) " << vehID << " created group ID: " << tsg.currentGroup << std::endl;
    }
    // Otherwise, request a group
    else {
        tsg.groupRequestsSent--;

        GroupRequest* wsm = new GroupRequest();
        wsm->setSender(vehID.c_str());
        wsm->setVehCoords(vehCoords.c_str());

        populateWSM(wsm);
        sendDown(wsm);
    }
}
void VeinsApp::finish()
{

//    if (tsg.isGroupLeader) {
        std::cout << "\n\t" << vehID << "'s score table:" << std::endl;
        tsg.printTrustSecureGroup();
        tsg.setTableStats();
        tsg.getTableStats();
//    }

//    std::cout << "\t" << vehID << " exited simulation in group: " << tsg.currentGroup << "\n\t\tGL: " << tsg.groupLeaders[tsg.currentGroup].first << " BGL: " << tsg.groupLeaders[tsg.currentGroup].second << std::endl;
    DemoBaseApplLayer::finish();
    // statistics recording goes here
}

void VeinsApp::onBSM(DemoSafetyMessage* bsm){
}

void VeinsApp::onWSM(BaseFrame1609_4* msg)
{
    // Your application has received a data message from another car or RSU

    std::string messageType = msg->getMessageType();

    if (messageType == "broadcast") {
        Broadcast* wsm = check_and_cast<Broadcast*>(msg);
        handleBroadcast(wsm);
    }
    else if (messageType == "trustlist") {
        TrustList* wsm = check_and_cast<TrustList*>(msg);
        handleTrustList(wsm);
    }
    else if (messageType == "grouprequest") {
        // Check if leader of a group
        if (tsg.isGroupLeader) {
            GroupRequest* wsm = check_and_cast<GroupRequest*>(msg);
            std::string sender = wsm->getSender();
            std::string incomingVehCoords = wsm->getVehCoords();

            // If request is within group range, admit the member to the group
            if (tsg.getEuclideanDistance(vehCoords, incomingVehCoords)) {
                {
                    GroupAcknowledge* ack = new GroupAcknowledge();

//                    std::cout << "\nSYN: Group acknowledgment for " << sender << " to join group " << tsg.currentGroup << " created by " << vehID << std::endl;
                    ack->setIntendedRecipient(sender.c_str());
                    ack->setSender(vehID.c_str());

                    ack->insertGroup(std::to_string(tsg.currentGroup).c_str());
                    ack->insertGroup(vehID.c_str());
                    ack->insertGroup(tsg.groupLeaders[tsg.currentGroup].second.c_str());

                    populateWSM(ack);
                    sendDown(ack);
                }
            }
        }
    }
    else if (messageType == "groupAcknowledge") {
        {
            GroupAcknowledge* wsm = check_and_cast<GroupAcknowledge*>(msg);

            // Check if intended recipient for group request and join group if so
            if (vehID == wsm->getIntendedRecipient()) {
//                std::cout << " ACK: " << wsm->getSender() << " accepted " << vehID << " to group " << wsm->getGroup(0) << std::endl;
                tsg.joinGroup(std::stoi(wsm->getGroup(0)), wsm->getGroup(1), wsm->getGroup(2));
            }
        }
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

    //Retrieve relevant self data
    Coord coords = mobility->getPositionAt(omnetpp::simTime());
    vehID = mobility->getExternalId();
    vehType = traciVehicle->getVType();
    vehSpeed = traciVehicle->getSpeed();
    timestamp = std::stod(omnetpp::simTime().str());
    vehCoords = formatCoords(coords.x, coords.y, 2);


    // Clear neighboring vehicles table of stale entries
    tsg.pruneNeighborTable(timestamp);

    // Request a group if one has not yet been joined
    if (tsg.currentGroup == -1 && tsg.groupRequestsSent <= 0 || tsg.currentGroup == -1 && (timestamp - initTime > 10)) {
//        std::cout << "\n(New Group) " << vehID << " issued a new group request: ";
        tsg.joinGroup(-1, vehID, "null");
        tsg.isGroupLeader = true;
    }

    // If group leader and no bgl is set, attempt to set a bgl
    if (tsg.isGroupLeader) {
        if (tsg.groupLeaders[tsg.currentGroup].second == "null") {
            std::string mostTrusted = tsg.getHighestTotalTrust();

            tsg.groupLeaders[tsg.currentGroup].second = mostTrusted;
        }
    }

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
        {
            Broadcast* wsm = new Broadcast();

            // Populate Broadcast with speed data
            wsm->setVehSpeed(traciVehicle->getSpeed());

            // Retrieve and set coordinate data
            Coord coords = mobility->getPositionAt(omnetpp::simTime());
            std::string vehCoords = formatCoords(coords.x, coords.y, 2);
            wsm->setVehCoords(vehCoords.c_str());

            //Set the vehID, speed, timestamp, vehType, and coordinate information for the vehicle's outbound messages
            wsm->setVehID(vehID.c_str());
            wsm->setVehType(vehType.c_str());

            //Populate group information
            if (tsg.isGroupLeader) {
                for (auto it = tsg.groupLeaders.begin(); it != tsg.groupLeaders.end(); ++it) {
                    const int& key = it->first;

                    wsm->insertGroups(std::to_string(key).c_str());
                    wsm->insertGroups(tsg.groupLeaders[key].first.c_str());
                    wsm->insertGroups(tsg.groupLeaders[key].second.c_str());
                }
            }

            // Send wsm with above data
            populateWSM(wsm);
            sendDown(wsm);
        }

        // Populate trustlist packet with current total observation trust data
        TrustList* wsm = new TrustList();

        for (auto it = tsg.totalObservationTable.begin(); it != tsg.totalObservationTable.end(); ++it) {
            wsm->insertTrustList((it->first).c_str());
            wsm->insertTrustList((it->second[2]).c_str());                                                  //NOTE: 2 corresponds with VEH_TRUSTSCORE_INDEX in tsg
        }

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

    std::string msgTime = omnetpp::simTime().str();

    // Unpack message data
    double incomingVehSpeed = wsm->getVehSpeed();
    std::string incomingVehType = wsm->getVehType();
    std::string incomingVehID = wsm->getVehID();
    std::string incomingVehCoords = wsm->getVehCoords();
    int incomingGroup = wsm->getVehGroup();

    // Expand 1-d array of groups & leader info [key: (group leader, backup group leader)]
    std::unordered_map<int, std::pair<std::string, std::string>> incomingGroupLeaders;
    int key;
    std::string gl;
    std::string bgl;

    for(int i = 0; i < wsm->getGroupsArraySize(); ++i) {
        if (i % 3 == 0) {
            key = std::stoi(wsm->getGroups(i));
        }
        else if (i % 3 == 1) {
            gl = wsm->getGroups(i);
        }
        else if (i % 3 == 2) {
            bgl = wsm->getGroups(i);
            incomingGroupLeaders[key] = std::make_pair(gl, bgl);
        }
    }

    // Verify coordinates fall within neighborhood range
    if (tsg.getEuclideanDistance(vehCoords, incomingVehCoords) <= GROUP_BROADCAST_RANGE) {
        // Request to join a group if not in one
        if (tsg.currentGroup == -1 && tsg.groupRequestsSent > 0) {
            tsg.groupRequestsSent--;

            {
                GroupRequest* wsm = new GroupRequest();
                wsm->setSender(vehID.c_str());
                wsm->setVehCoords(vehCoords.c_str());

                populateWSM(wsm);
                sendDown(wsm);
            }
        }


        // Sender was GL -- update GL information
        if (tsg.groupLeaders[tsg.currentGroup].first == incomingVehID) {
            tsg.groupLeaders[tsg.currentGroup].second = incomingGroupLeaders[tsg.currentGroup].second;
        }
        // Sender was BGL and no GL assigned
        else if (tsg.groupLeaders[tsg.currentGroup].second == incomingVehID && tsg.groupLeaders[tsg.currentGroup].first == "null") {
            tsg.groupLeaders[tsg.currentGroup].first = tsg.groupLeaders[tsg.currentGroup].second;
        }

        // GL doesn't exist and recipient is BGL
        if (tsg.isBackupGroupLeader && tsg.groupLeaders[tsg.currentGroup].first == "null") {
            tsg.groupLeaders[tsg.currentGroup].first = vehID;
            tsg.groupLeaders[tsg.currentGroup].second = tsg.getHighestTotalTrust();
            tsg.isBackupGroupLeader = false;
            tsg.isGroupLeader = true;
        }
        // GL appointed vehicle as BGL
        if (incomingVehID == tsg.groupLeaders[tsg.currentGroup].first && tsg.groupLeaders[tsg.currentGroup].second == vehID) {
            tsg.isBackupGroupLeader = true;
        }

        // Process incoming vehicle information
        tsg.handleBroadcast(&incomingVehID, &incomingVehCoords, &incomingVehSpeed, &msgTime, &incomingVehType, &roadSpeedLimit, &incomingGroupLeaders);
    }

}

void VeinsApp::handleTrustList(TrustList* wsm) {
    std::unordered_map<std::string, double> trustList;
    std::string key;
    double value;

    // Expand 1-d array data to a map of vehicle:trust pairs
    for (int i = 0; i < wsm->getTrustListArraySize(); ++i) {
        if (i % 2 == 0) {
            key = wsm->getTrustList(i);
        }
        else if (i % 2 == 1) {
            value = std::stod(wsm->getTrustList(i));
            trustList[key] = value;

//            std::cout <<"(Trust MSG) "<< vehID << " received " << key << ": " << value << std::endl;
        }
    }

    tsg.handleTrustList(&trustList);
}

