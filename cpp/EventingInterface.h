/*
 * EventingInterface.h
 *
 *  Created on: Mar 20, 2015
 *      Author: ylbagou
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
