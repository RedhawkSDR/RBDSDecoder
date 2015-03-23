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


#ifndef RBDSDECODER_IMPL_H
#define RBDSDECODER_IMPL_H

#include "RBDSDecoder_base.h"
#include "EventingInterface.h"
#include "LoggingInterface.h"
#include "gr-rds/decoder_impl.h"
#include "rds_constants.h"

class RBDSDecoder_i;
using namespace GenericInterface;

class RBDSDecoder_i : public RBDSDecoder_base, public virtual EventingInterface, public virtual LoggingInterface
{
    ENABLE_LOGGING
    public: 
        RBDSDecoder_i(const char *uuid, const char *label);
        ~RBDSDecoder_i();
        int serviceFunction();
        void log(enum LoggingInterface::LEVEL level, std::string msg);
        void sendMessage(enum TYPE eventType, std::string groupType, std::string msg);
        void reset(void);
    private:
    	long chanrf, colrf;
    	void checkForFreqChange(BULKIO::StreamSRI &sri);
    	void send_message(char type, bool version_code);
    	decoder_impl decoder;
    	std::string m_alt_freq, m_clock_time, m_flag_string, m_ps, m_radio_text, m_pi_txt;
    	unsigned int m_pty, m_pi;

        // Function to get an SRI keyword value
        template <typename TYPE> TYPE getKeywordByID(BULKIO::StreamSRI &sri, CORBA::String_member id, bool &valid) {
                /****************************************************************************************************
                 * Description: Retrieve the value assigned to a given id.
                 * sri   - StreamSRI object to process
                 * id    - Keyword identifier string
                 * valid - Flag to indicate whether the returned value is valid (false if the keyword doesn't exist)
                 ****************************************************************************************************/
                valid = false;
                TYPE value;

                for(unsigned int i=0; i < sri.keywords.length(); i++) {
                        if(!strcmp(sri.keywords[i].id, id)) {
                                valid = true;
                                if (sri.keywords[i].value >>= value)
                                        break;
                                //try with double and float to extract it and see if we can make it happen if the
                                //format of this keyword is different than we expect
                                double d;
                                if (sri.keywords[i].value >>= d)
                                {
                                        value=static_cast<TYPE>(d);
                                        break;
                                }
                                float f;
                                if (sri.keywords[i].value >>= f)
                                {
                                        value=static_cast<TYPE>(f);
                                        break;
                                }
                                valid = false;
                        }
                }
                return value;
        }
};

#endif
