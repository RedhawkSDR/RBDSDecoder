/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK RBDSDecoder.
 *
 * REDHAWK RBDSDecoder is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK RBDSDecoder is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef EVENTINGINTERFACE_H_
#define EVENTINGINTERFACE_H_

namespace GenericInterface {

class EventingInterface {
public:
	/* type 0 = PI
	 * type 1 = PS
	 * type 2 = PTY
	 * type 3 = flagstring: TP, TA, MuSp, MoSt, AH, CMP, stPTY
	 * type 4 = RadioText
	 * type 5 = ClockTime
	 * type 6 = Alternative Frequencies */
	enum TYPE {PI, PS, PTY, FLAG_STRING, RADIO_TEXT, CLOCK_TIME, ALT_FREQ};
	virtual void sendMessage(enum TYPE eventType, std::string groupType, std::string msg) = 0;
};

}

#endif /* EVENTINGINTERFACE_H_ */
