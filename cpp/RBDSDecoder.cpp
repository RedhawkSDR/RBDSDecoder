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

#include "RBDSDecoder.h"
#include "rds_constants.h"

PREPARE_LOGGING(RBDSDecoder_i)

RBDSDecoder_i::RBDSDecoder_i(const char *uuid, const char *label) :
		EventingInterface(),
		LoggingInterface(),
		RBDSDecoder_base(uuid, label),
		chanrf(0),
		colrf(0),
		decoder(this, this),
		m_alt_freq(""), m_clock_time(""), m_flag_string(""), m_ps(""), m_radio_text(""), m_pi_txt(""), m_pty(0), m_pi(0)
{
}

RBDSDecoder_i::~RBDSDecoder_i() {
}

void RBDSDecoder_i::sendMessage(enum TYPE eventType, std::string groupType, std::string msg) {
	std::string group;

	switch (eventType) {
	case EventingInterface::ALT_FREQ:
		LOG_INFO(RBDSDecoder_i, "Received unsupported ALT FREQ(" << msg << ") event type.  Ignoring.")
		m_alt_freq = msg;
		return;
		break;
	case EventingInterface::CLOCK_TIME:
		LOG_INFO(RBDSDecoder_i, "Received unsupported CLOCK TIME(" << msg << ") event type.  Ignoring.")
		m_clock_time = msg;
		return;
		break;
	case EventingInterface::FLAG_STRING:
		LOG_TRACE(RBDSDecoder_i, "Received FLAG_STRING message type." << msg)
		m_flag_string = msg;
		break;
	case EventingInterface::PI:
		LOG_TRACE(RBDSDecoder_i, "Received PI message type:" << msg)
		try {
			m_pi_txt = msg;
			m_pi = atoi(msg.c_str());
			LOG_FATAL(RBDSDecoder_i, "Converted m_pi:" << m_pi)
		} catch(std::exception const & e) {
			LOG_ERROR(RBDSDecoder_i, "Error converting PTY from string to int");
		}
		break;
	case EventingInterface::PS:
		LOG_TRACE(RBDSDecoder_i, "Received PS message type: " << msg)
		m_ps = msg;
		break;
	case EventingInterface::PTY:
		LOG_TRACE(RBDSDecoder_i, "Received PTY message type: " << msg)
		try {
			unsigned int tmp;
			tmp = atoi(msg.c_str());
			if ((tmp >= 0) & (tmp < 32)) {
				m_pty = tmp;
			} else {
				LOG_WARN(RBDSDecoder_i, "Received PTY not within valid range");
			}
		} catch(std::exception const & e) {
			LOG_ERROR(RBDSDecoder_i, "Error converting PTY from string to int");
		}

		break;
	case EventingInterface::RADIO_TEXT:
		LOG_TRACE(RBDSDecoder_i, "Received RADIO_TEXT message type: " << msg)
		m_radio_text = msg;
		break;
	default:
		LOG_WARN(RBDSDecoder_i, "Received unknown message type: " << msg)
	}

	RBDS_Output_struct mess;
	mess.Call_Sign = decode_callsign(m_pi);
	mess.Full_Text = m_radio_text;
	mess.Group = groupType;
	mess.PI_String = m_pi_txt;
	mess.Short_Text = m_ps;
	mess.Station_Type = rbds_pty_table[m_pty];
	mess.TextFlag = m_flag_string;

	messageEvent_out->sendMessage(mess);
}
void RBDSDecoder_i::log(enum LoggingInterface::LEVEL level, std::string msg) {

	switch (level) {
	case LoggingInterface::TRACE:
		LOG_TRACE(RBDSDecoder_i, msg);
		break;
	case LoggingInterface::INFO:
		LOG_INFO(RBDSDecoder_i, msg);
		break;
	case LoggingInterface::DEBUG:
		LOG_DEBUG(RBDSDecoder_i, msg);
		break;
	case LoggingInterface::WARN:
		LOG_WARN(RBDSDecoder_i, msg);
		break;
	case LoggingInterface::ERROR:
		LOG_ERROR(RBDSDecoder_i, msg);
		break;
	case LoggingInterface::FATAL:
		LOG_FATAL(RBDSDecoder_i, msg);
		break;
	default:
		break;
	}
}

void RBDSDecoder_i::checkForFreqChange(BULKIO::StreamSRI &sri) {
	long tmpColRf = 0;
	long tmpChanRf = 0;
	bool validCollectionRF = false;
	bool validChannelRF = false;

	long collection_rf = getKeywordByID<CORBA::Long>(sri, "COL_RF", validCollectionRF);
	long channel_rf = getKeywordByID<CORBA::Long>(sri, "CHAN_RF", validChannelRF);

	if ((validCollectionRF) && (validChannelRF)) {
		tmpColRf = collection_rf;
		tmpChanRf = channel_rf;
		LOG_INFO(RBDSDecoder_i, "COL_RF Changed from: " << colrf << " to: " << tmpColRf);
		LOG_INFO(RBDSDecoder_i, "CHAN_RF Changed from: " << chanrf << " to: " << tmpChanRf);
	} else if (validCollectionRF) {
		tmpColRf = collection_rf;
		LOG_INFO(RBDSDecoder_i, "COL_RF Changed from: " << colrf << " to: " << tmpColRf);
	} else if (validChannelRF) {
		tmpChanRf = channel_rf;
		LOG_INFO(RBDSDecoder_i, "CHAN_RF Changed from: " << chanrf << " to: " << tmpChanRf);
	} else {
		tmpChanRf = 0;
		tmpColRf = 0;
	}

	if (colrf != tmpColRf || chanrf != tmpColRf) {
		colrf = tmpColRf;
		chanrf = tmpChanRf;
		decoder.reset();
	}

}

int RBDSDecoder_i::serviceFunction() {
	bulkio::InShortPort::dataTransfer * input = dataShort_in->getPacket(-1);

	if (not input) {
		return NOOP;
	}
	std::vector<short> bitsIn = input->dataBuffer;

	if (input->sriChanged) {
		checkForFreqChange(input->SRI);
	}

	decoder.work(bitsIn);

	delete input;

	return NORMAL;
}


