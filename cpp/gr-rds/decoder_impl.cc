/**
 * This file is a modified version of the file found in the gr-rds project here:
 * https://github.com/bastibl/gr-rds
 * Please see the github project for the original version as well as the chain of
 * forks & ownership.
 *
 * It has been modified to remove the dependency on gnu radio so that it may be used with the
 * REDHAWK framework.
 */


/*
 * Copyright (C) 2014 Bastian Bloessl <bloessl@ccs-labs.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "decoder_impl.h"
#include "constants.h"

decoder_impl::decoder_impl(GenericInterface::LoggingInterface *logger, GenericInterface::EventingInterface * eventServer):
parser(logger, eventServer)
{
	this->logger = logger;
	this->eventServer = eventServer;
	reset();
}

decoder_impl::~decoder_impl() {
}

/**
 * Externally callable method to enter_no_sync() and reset the parser.
 */
void decoder_impl::reset() {
	parser.reset();
	enter_no_sync();
}

////////////////////////// HELPER FUNTIONS /////////////////////////

void decoder_impl::enter_no_sync() {
	presync = false;
	d_state = NO_SYNC;
}

void decoder_impl::enter_sync(unsigned int sync_block_number) {
	wrong_blocks_counter   = 0;
	blocks_counter         = 0;
	block_bit_counter      = 0;
	block_number           = (sync_block_number + 1) % 4;
	group_assembly_started = false;
	d_state                = SYNC;
}

/* see Annex B, page 64 of the standard */
unsigned int decoder_impl::calc_syndrome(unsigned long message, unsigned char mlen) {
	unsigned long reg = 0;
	unsigned int i;
	const unsigned long poly = 0x5B9;
	const unsigned char plen = 10;

	for (i = mlen; i > 0; i--)  {
		reg = (reg << 1) | ((message >> (i-1)) & 0x01);
		if (reg & (1 << plen)) reg = reg ^ poly;
	}
	for (i = plen; i > 0; i--) {
		reg = reg << 1;
		if (reg & (1<<plen)) reg = reg ^ poly;
	}
	return (reg & ((1<<plen)-1));	// select the bottom plen bits of reg
}

void decoder_impl::decode_group(unsigned int *group) {
	parser.parse(group);
}

void decoder_impl::work (std::vector<short> &in) {

	int i=0,j;
	unsigned long bit_distance, block_distance;
	unsigned int block_calculated_crc, block_received_crc, checkword,dataword;
	unsigned int reg_syndrome;

/* the synchronization process is described in Annex C, page 66 of the standard */
	while (i < in.size()) {
		reg=(reg<<1)|in[i];		// reg contains the last 26 rds bits
		switch (d_state) {
			case NO_SYNC:
				reg_syndrome = calc_syndrome(reg,26);
				for (j=0;j<5;j++) {
					if (reg_syndrome==syndrome[j]) {
						if (!presync) {
							lastseen_offset=j;
							lastseen_offset_counter=bit_counter;
							presync=true;
						}
						else {
							bit_distance=bit_counter-lastseen_offset_counter;
							if (offset_pos[lastseen_offset]>=offset_pos[j]) 
								block_distance=offset_pos[j]+4-offset_pos[lastseen_offset];
							else
								block_distance=offset_pos[j]-offset_pos[lastseen_offset];
							if ((block_distance*26)!=bit_distance) presync=false;
							else {
								LOG(GenericInterface::LoggingInterface::TRACE, "@@@@@ Sync State Detected");
								enter_sync(j);
							}
						}
					break; //syndrome found, no more cycles
					}
				}
			break;
			case SYNC:
/* wait until 26 bits enter the buffer */
				if (block_bit_counter<25) block_bit_counter++;
				else {
					good_block=false;
					dataword=(reg>>10) & 0xffff;
					block_calculated_crc=calc_syndrome(dataword,16);
					checkword=reg & 0x3ff;
/* manage special case of C or C' offset word */
					if (block_number==2) {
						block_received_crc=checkword^offset_word[block_number];
						if (block_received_crc==block_calculated_crc)
							good_block=true;
						else {
							block_received_crc=checkword^offset_word[4];
							if (block_received_crc==block_calculated_crc)
								good_block=true;
							else {
								wrong_blocks_counter++;
								good_block=false;
							}
						}
					}
					else {
						block_received_crc=checkword^offset_word[block_number];
						if (block_received_crc==block_calculated_crc)
							good_block=true;
						else {
							wrong_blocks_counter++;
							good_block=false;
						}
					}
/* done checking CRC */
					if (block_number==0 && good_block) {
						group_assembly_started=true;
						group_good_blocks_counter=1;
					}
					if (group_assembly_started) {
						if (!good_block) group_assembly_started=false;
						else {
							group[block_number]=dataword;
							group_good_blocks_counter++;
						}
						if (group_good_blocks_counter==5) decode_group(group);
					}
					block_bit_counter=0;
					block_number=(block_number+1) % 4;
					blocks_counter++;
/* 1187.5 bps / 104 bits = 11.4 groups/sec, or 45.7 blocks/sec */
					if (blocks_counter==50) {
						if (wrong_blocks_counter>35) {
							LOG(GenericInterface::LoggingInterface::TRACE, "@@@@@ Lost Sync (Got " << wrong_blocks_counter
									<< " bad blocks on " << blocks_counter
									<< " total)");
							enter_no_sync();
						} else {
							LOG(GenericInterface::LoggingInterface::TRACE, "@@@@@ Still Sync-ed (Got " << wrong_blocks_counter
									<< " bad blocks on " << blocks_counter
									<< " total)");
						}
						blocks_counter=0;
						wrong_blocks_counter=0;
					}
				}
			break;
			default:
				d_state=NO_SYNC;
			break;
		}
		i++;
		bit_counter++;
	}
}
