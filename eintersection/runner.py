#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
# Copyright (C) 2009-2023 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    runner.py
# @author  Lena Kalleske
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2009-03-26

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import optparse
import random
import re

# we need to import python modules from the $SUMO_HOME/tools directory
if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

from sumolib import checkBinary  # noqa
import traci  # noqa


def generate_routefile():
    random.seed(42)  # make tests reproducible
    N = 3600  # number of time steps
    # demand per second from different directions
    pWE = 1. / 20
    pEW = 1. / 21
    pNS = 1. / 20
    pSN = 1. / 25
    
    with open("data/cross.rou.xml", "w") as routes:
    	#vType modified lc data to prevent lane changes; routes modified to allow E->W, N->S only
        print("""<routes>
        <vType id="typeWE" accel="0.8" decel="4.5" sigma="0.5" length="5" minGap="2.5" maxSpeed="15" vClass="passenger" guiShape="passenger" lcSpeedGain="0" lcKeepRight="0" lcCooperative="0"/>
        <vType id="typeNS" accel="0.8" decel="4.5" sigma="0.5" length="5" minGap="3" maxSpeed="15" vClass="passenger" guiShape="passenger" lcSpeedGain="0" lcKeepRight="0" lcCooperative="0"/>
        
        <vType id="emerWE" accel="0.8" decel="4.5" sigma="0.5" length="7" minGap="2.5" maxSpeed="25" vClass="emergency" guiShape="emergency" lcSpeedGain="0" lcKeepRight="0" lcCooperative="0"/>
        <vType id="emerNS" accel="0.8" decel="4.5" sigma="0.5" length="7" minGap="2.5" maxSpeed="25" vClass="emergency" guiShape="emergency" lcSpeedGain="0" lcKeepRight="0" lcCooperative="0"/>

        <route id="right" edges="51o 1i 2o 52i" />
        <route id="down" edges="54o 4i 3o 53i" />""", file=routes)
        vehNr = 0
        for i in range(N):
            isEmergency = False
            color = "0,0,1"
            if random.randint(0, 100) < 10:
            	isEmergency = True
            	color = "1,0,0"
            
            if random.uniform(0, 1) < pWE:
                vtype = "emerWE" if isEmergency else "typeWE"
                print('\t<vehicle id="right_%i" type="%s" route="right" depart="%i" color="%s"/>' % (vehNr, vtype, i, color), file=routes)
                vehNr += 1
            if random.uniform(0, 1) < pNS:
            	vtype = "emerNS" if isEmergency else "typeNS"
            	print('\t<vehicle id="down_%i" type="%s" route="down" depart="%i" color="%s"/>' % (vehNr, vtype, i, color), file=routes)
            	vehNr += 1
            
        print("</routes>", file=routes)

def getTLSDistance(vehID):
    """TraCI claims that .getNextTLS returns a string, but it actually returns a tuple.
    This converts the tuple to a string, checks if the 'tuple' is empty and returns inf for distance if it is, 
    and extracts the distance to the next traffic light as a float if it is not empty.
    """
    nextTLS = str(traci.vehicle.getNextTLS(vehID))
    if nextTLS == "()":
        dist = float('inf')
    else:
        dist = float(nextTLS.split(',')[2].strip())

    return dist

def run():
    """execute the TraCI control loop"""
    step = 0
    lightPhase = 0 # Tracks phase change

    while traci.simulation.getMinExpectedNumber() > 0:
        traci.simulationStep()
        """
        # Holds all stopped emergency vehicle's distance to light & travel route (distToTLS, vehicleType)
        waitingEMS = []
             
        # Check all vehicles in simulation for proximity to light, stop status, and emergency vehicle type
        vehsInSim= traci.vehicle.getIDList()
        for vehID in vehsInSim:
            vehSpeed = traci.vehicle.getSpeed(vehID)	# Vehicle travel speed
            vehType = traci.vehicle.getTypeID(vehID)	# Vehicle type identifier - emergency or passenger
            distToTLS = getTLSDistance(vehID)		# Vehicle proximity to traffic lights
            
            # If an emergency vehicle is waiting at intersection, add (distance, vehicleType) to list
            if vehSpeed < 0.01 and distToTLS < 100 and re.search("emer*", vehType):
            	waitingEMS.append((distToTLS, vehType))
            	        
        # Sort waiting vehicles by distance to determine light priority
        waitingEMS.sort()     
        
        # If an emergency vehicle is waiting in queue, change the light
        if len(waitingEMS) > 0 and re.search(".*NS", waitingEMS[0][1]):
            traci.trafficlight.setPhase("0", 0)
        elif len(waitingEMS) > 0 and re.search(".*WE", waitingEMS[0][1]):
            traci.trafficlight.setPhase("0", 2)
        else:
            #This is a gross bit of code, but you can't take the modulo of a number to get either 2 and 0
            #So every 30 steps we check the light phase and alternate it with a ternary
            if step % 30 == 0:
                lightPhase = 0 if lightPhase == 2 else 2
                traci.trafficlight.setPhase("0", lightPhase)
                """
        step += 1
    traci.close()
    sys.stdout.flush()


def get_options():
    optParser = optparse.OptionParser()
    optParser.add_option("--nogui", action="store_true",
                         default=False, help="run the commandline version of sumo")
    options, args = optParser.parse_args()
    return options


# this is the main entry point of this script
if __name__ == "__main__":
    options = get_options()

    # this script has been called from the command line. It will start sumo as a
    # server, then connect and run
    if options.nogui:
        sumoBinary = checkBinary('sumo')
    else:
        sumoBinary = checkBinary('sumo-gui')

    # first, generate the route file for this simulation
    generate_routefile()

    # this is the normal way of using traci. sumo is started as a
    # subprocess and then the python script connects and runs
    traci.start([sumoBinary, "-c", "data/cross.sumocfg",
                             "--tripinfo-output", "tripinfo.xml"])
    run()
