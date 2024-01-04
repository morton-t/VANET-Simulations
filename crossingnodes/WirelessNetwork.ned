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

package sybil.simulations.crossingnodes;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.node.inet.WirelessHost;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;

//
// TODO auto-generated type
//
network WirelessNetwork
{
    parameters:

        @display("bgb=1117.4,858.888");
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            @display("p=572.368,49.996");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=86.2,49.996");
        }
        radioMedium: <default("UnitDiskRadioMedium")> like IRadioMedium {
            @display("p=1029.228,48.272");
        }
        hostA: WirelessHost {
            @display("p=20.536,465.08");
        }
        hostB: WirelessHost {

            @display("p=1092.032,327.368");
        }
}
