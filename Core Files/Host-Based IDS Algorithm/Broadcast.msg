//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

//
// TODO generated message class
//

import veins.base.utils.Coord;
import veins.modules.messages.BaseFrame1609_4;
import veins.base.utils.SimpleAddress;

namespace veins;


packet Broadcast extends BaseFrame1609_4 {
    //Message for vehicle to vehicle broadcasts of a single vehicle's own traffic information

	//Essential variables for HostBased system
    string vehSpeed;
    string vehDensity;
    string vehFlow;
    
    //Additional variables for statistical modelling
    string vehType;			// For assessment of correct prediction
    string vehID;			// For identifying vehicle without using MAC/IP/Pseudonyms etc
    string vehCoords;		// To extract coordinate data without signal analysis

    //LAddress::L2Type senderAddress = -1;
    //int serial = 0;
}