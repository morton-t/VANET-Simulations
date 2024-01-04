/*
 * To Build
 * 1.) right-click omnetpp.ini -> Run As -> Run Configurations
 * 2.) Select the simulation on LHS tree menu (eintersection for this case)
 * 3.) Ensure working dir, ini file, config name, # processors
 * 4.) User interface (QTEnv or TKEnv)
*/

//#include "Tools.h"
#include "omnetpp.h"
#include "string.h"
//#include "TraCICommandInterface.h"
//#include "TraCIScenarioManager.h"
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::AnnotationManager;

using namespace omnetpp;

class Tools : public BaseWaveApplLayer{
public:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
//    void Tools();
//    void ~Tools();
//    void initialize();
//    void handleMessage();
//    void SetVType();
//    void SendBroadcast();
//    friend void PassengerModule::handleMessage();
protected:
    TraCIMobility* mobility;
    TraCICommandInterface* traci;
    TraCICommandInterface::Vehicle* traciVehicle;
    AnnotationManager* annotations;
};

Define_Module(Tools);
/*
void Tools::Tools() {
    cout << "Instantiated Tools" << endl;
}
void Tools::~Tools() {
    //delete(Tools);
}
*/
void Tools::initialize(int stage) {
    if (stage == 0) {
        BaseWaveApplLayer::initialize();
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
    }
    //Sets default values for Tools.h
    //paramName = par("paramName") for paramName in .ned file
    //cMessage *msg = new cMessage("Message");
    //send(msg, "out");
}

void Tools::onData(WaveShortMessage* wsm) {
    findHost()->getDisplayString().updateWith("r=16,green");
    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobility->getPositionAt(simTime()), "blue"));

    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getWsmData(), 9999);
    if (!sentMessage) sendMessage(wsm->getWsmData());
}

void Tools::sendMessage(std::string blockedRoadId) {
    sentMessage = true;

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}

void Tools::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}

void Tools::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking) return;
    sendDelayedDown(wsm,individualOffset);
}

void Tools::handleMessage(cMessage *msg) {
    delete msg;
    cMessage *new_msg = new cMessage("Message");
    send(new_msg, "out");
}
/*
void SetVType() {
    const char* type = traci.vehicle->getVehicleClass();
    if (strcmp(type, "emergency") == 0) {
        //TODO Code for emergency vehicle signaling in .msg file
    }
    else {
        //TODO Code for passenger vehicle signaling
    }
}

void SendBroadcast() {
    MyMessage *msg = new MyMessage();
    msg->setBroadcast(true);
    send(msg, "out");
}

void PassengerModule::handleMessage(cMessage *msg){
    //Setup TraCI Mobility Module
    traci = traCIMobilityAccess().get(getParentModule());

    const char* senderVType;
    const char* receiverVType;

    //Retrieve ID of vehicle receiving message
    const int VID = traci->getExternalId();

    //Cast message type
    MyMessage *receivedMsg = check_and_cast<MyMessage *>(msg);

    //Get vehicle types for sender and receiver
    senderVType = receivedMsg->vehicleType;
    receiverVType = traci.vehicle->getVehicleClass();

    //Execute lane change for vehicle if sender is of type `emergency` and receiver is not
    if (strcmp(senderVType, "emergency") == 0 && strcmp(receiverVType, "emergency") != 0) {
        //Given VID attempts lane changes +/- 1 lane for 30 seconds
        traci.vehicle.changeLaneRelative(receiverVID, 1, 30);
    }
}
*/
