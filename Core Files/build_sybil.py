from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import optparse
import random

def build_route(routes, vehNr, vtype, i, route, lane, vcolor):
    print('\t<vehicle id="%s_%i" type="%s" route="%s" depart="%i" departLane="%s" color="%s"/>' % (route, vehNr, vtype, route, i, lane, vcolor), file=routes)

def generate_routefile():
    random.seed(42)  # make tests reproducible
    N = 3600  # number of time steps
    MAX = 50 # number of cars in simulation
    PERCENT = .40 # percent of ghosts in simulation
    # demand per second from different directions
    START_ATTACK = 5 # vehicle number in which ghosts begin generating
    density_south = 1. / 6
    density_east = 1. / 6
    
    with open("cross.rou.xml", "w+") as routes:
    	#vType modified lc data to prevent lane changes
        print("""
<!-- Route contains %i vehicles and %f malicious nodes -->

<routes>
        <vType id="passenger" accel="35" decel="0.1" sigma="0" length="5" minGap="2.5" maxSpeed="35" vClass="passenger" guiShape="passenger"/>
        <vType id="attacker" accel="5" decel="0.1" sigma="0" length="5" minGap="2.5" maxSpeed="5" vClass="passenger" guiShape="passenger" lcSpeedGain="0" lcKeepRight="0" lcCooperative="0"/>
        <vType id="ghost" accel="5" decel="0.1" sigma="0" length="5" minGap="2.5" maxSpeed="5" vClass="passenger" guiShape="passenger" lcSpeedGain="0" lcKeepRight="0" lcCooperative="0"/>

        <route id="right" edges="1i 2o" />
        <route id="down" edges="4i 3o" />""" % (MAX, PERCENT), file=routes)
        
        
        # Upkeep variables for loop
        i = 0
        vehNr = 0
        numGhosts = 0
        vtype = "passenger"
        vcolor = "0,0,1" ## Blue
        attackerRoute = ""
        
        while i < N:
            if vehNr == MAX:
                break

	    # Generate route for attacker
            if attackerRoute == "":
                if random.uniform(0,1) < .5:
                    attackerRoute = "pWE"
                else:
                    attackerRoute = "pNS"
                    
            # Set up for generating ghosts at START_ATTACK number of vehicles
            if vehNr == START_ATTACK:
                vtype = "ghost"
                numGhosts = (int)(MAX * PERCENT)
                
            # Generate ghosts on the given route
            if vtype == "ghost":
                vcolor = "1,1,1"  ## white
                
                while numGhosts > 0:
                    match attackerRoute:
                        case "pWE":
                            build_route(routes, vehNr, vtype, i, 'right', '0', vcolor)
                        case "pNS":
                            build_route(routes, vehNr, vtype, i, 'down', '0', vcolor)
                        case _:
                            break
                    i += 1        
                    vehNr +=1        
                    numGhosts -= 1
                
                # Set vtype to generate an attacker once ghosts are created    
                vtype = "attacker"
                continue
            
            elif vtype == "attacker":
                vcolor = "1,0,0" # red
                match attackerRoute:
                        case "pWE":
                            build_route(routes, vehNr, vtype, i, 'right', '0', vcolor)
                        case "pNS":
                            build_route(routes, vehNr, vtype, i, 'down', '0', vcolor)
                        case _:
                            break
                
                # Set vtype back to passenger once attacker is created
                attackerRoute = ""
                vtype = "passenger"
                vcolor = "0,0,1" # blue
                
                vehNr += 1
                
            # Generate vehicles based on random distribution
            else:
                if random.uniform(0, 1) < density_east:
                    build_route(routes, vehNr, vtype, i, 'right', 'random', vcolor)
                    vehNr += 1
                    
                if random.uniform(0, 1) < density_south:
                    build_route(routes, vehNr, vtype, i, 'down', 'random', vcolor)
                    vehNr += 1
                
            i += 1
              
        print("</routes>", file=routes)
        routes.close()

# this is the main entry point of this script
if __name__ == "__main__":
    generate_routefile()
else:
    print('Please run this file as a standalone!')
    
