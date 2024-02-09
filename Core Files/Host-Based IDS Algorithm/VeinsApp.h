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

#pragma once

#include "veins/veins.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "stdlib.h"
#include "string.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "Broadcast_m.h"

#include "HostBased.h"

static const double TRANSMISSION_RANGE = 500; // Transmission range in meters - 802.11p range is 300-1000m; 500 was selected for this simulation


using namespace omnetpp;


namespace veins {

/**
 * @brief
 * This is a stub for a typical Veins application layer.
 * Most common functions are overloaded.
 * See MyVeinsApp.cc for hints
 *
 * @author David Eckhoff
 *
 */

class VEINS_API VeinsApp : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;
    void finish() override;

    HostBased st; /** Score Table containing traffic data */

    bool init; /** On init, vehicles report speed, coords, etc. which results in inaccurate table info. Ensures vehs are moving before reporting data */


protected:
    std::string vehID;
    std::string vehType;

    void onBSM(DemoSafetyMessage* bsm) override;
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;

    std::string formatCoords(double x, double y, int precision);

    void handleBroadcast(Broadcast* wsm);
};

} // namespace veins
