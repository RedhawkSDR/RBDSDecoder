/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK.
 *
 * REDHAWK is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef RBDSDECODER_IMPL_H
#define RBDSDECODER_IMPL_H

#include "RBDSDecoder_base.h"
//#include "tmc_events.h"

class RBDSDecoder_i;

class RBDSDecoder_i : public RBDSDecoder_base
{
    ENABLE_LOGGING
    public: 
        RBDSDecoder_i(const char *uuid, const char *label);
        ~RBDSDecoder_i();
        int serviceFunction();
        void reset(void);
    private:
    	unsigned long bit_counter, lastseen_offset_counter, reg;
    	unsigned char lastseen_offset, block_number;
    	unsigned int block_bit_counter, wrong_blocks_counter, blocks_counter, group_good_blocks_counter;
    	bool presync, good_block, group_assembly_started;

    	unsigned int group[4];
    	enum state_t {
    		ST_NO_SYNC, ST_SYNC
    	};
    	state_t d_state;
    	long chanrf, colrf;
    	char clocktime_string[33];
    	char radiotext[65];
    	char af1_string[10];
    	char af2_string[10];
    	char af_string[21];
    	char pistring[5];
    	char groupID[3];
    	char radiotext_flag;
    	char pi_country_identification;
    	char pi_area_coverage;
    	char pi_program_reference_number;
    	bool radiotext_AB_flag;
    	bool traffic_program;
    	bool traffic_announcement;
    	bool music_speech;
    	bool mono_stereo;
    	bool artificial_head;
    	bool compressed;
    	bool static_pty;
    	unsigned char program_type;
    	unsigned int program_identification;
    	char program_service_name[9];
    	char callsign[5];

    	std::map<unsigned int, std::string> callMap;

    	void enter_no_sync();
    	void checkForFreqChange(BULKIO::StreamSRI &sri);
    	void enter_sync(unsigned int);
    	void reset_rds_data();
    	unsigned int calc_syndrome(unsigned long, unsigned char);
    	void decode_group(unsigned int*);
    	double decode_af(unsigned int);
    	void decode_type0(unsigned int*, bool);
    	void decode_type1(unsigned int*, bool);
    	void decode_type2(unsigned int*, bool);
    	void decode_type3a(unsigned int*);
    	void decode_type4a(unsigned int*);
    	void decode_type8a(unsigned int*);
    	void decode_type14(unsigned int*, bool);
    	void decode_type15b(unsigned int*);
    	void decode_callsign(unsigned int);
    	void set_call_map();
    	void send_message(char type, bool version_code);
    	void decode_optional_content(int no_groups, unsigned long int *free_format);

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
