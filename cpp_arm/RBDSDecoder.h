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
    	char radiotext[65];
    	char af1_string[10];
    	char af2_string[10];
    	char af_string[21];
    	char pistring[5];
    	char groupID[3];
    	char radiotext_flag;
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
    	void enter_sync(unsigned int);
    	void reset_rds_data();
    	unsigned int calc_syndrome(unsigned long, unsigned char);
    	void decode_group(unsigned int*);
    	double decode_af(unsigned int);
    	void decode_optional_content(int, unsigned long int *);
    	void decode_type0(unsigned int*, bool);
    	void decode_type2(unsigned int*, bool);
    	void decode_callsign(unsigned int);
    	void set_call_map();
};

#endif
