/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.

 * This file is part of RBDSDecoder.

 * RBDSDecoder is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.

 * RBDSDecoder is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

 	Source: RBDSDecoder.spd.xml
 	Generated on: Thu Aug 15 09:30:12 EDT 2013
 	REDHAWK IDE
 	Version: 1.8.4
 	Build id: R201305151907

**************************************************************************/

#include "RBDSDecoder.h"
#include "rds_constants.h"

PREPARE_LOGGING(RBDSDecoder_i)

RBDSDecoder_i::RBDSDecoder_i(const char *uuid, const char *label) : 
    RBDSDecoder_base(uuid, label)
{
	//Sets Debug level so that the LOG_INFO does not display. Set this to
	//3 or above to view LOG_INFO
	LoggingConfigurator::configure(2);

	//set callMap for 3 letter callsigns
	set_call_map();
	reset();
}

RBDSDecoder_i::~RBDSDecoder_i()
{
}

int RBDSDecoder_i::serviceFunction()
{
	bulkio::InShortPort::dataTransfer * input = dataIn->getPacket(-1);

	if (not input) {
		return NOOP;
	}

	const short *in = (const short *) input->dataBuffer.data();

	unsigned int i = 0, j;
    unsigned long bit_distance, block_distance;
    unsigned int block_calculated_crc, block_received_crc, checkword, dataword;
    unsigned int reg_syndrome;

	while (i < input->dataBuffer.size()) {
		reg = (reg << 1) | (in[i] ? 0x1 : 0x0); // reg contains the last 26 rds bits
		switch (d_state) {
			case ST_NO_SYNC: {
				reg_syndrome = calc_syndrome(reg, 26);

				for (j = 0; j < 5; j++) {
					if (reg_syndrome == syndrome[j]) {
						if (!presync) {
							lastseen_offset = j;
							lastseen_offset_counter = bit_counter;
							presync = true;
						}
						else {
							bit_distance = bit_counter - lastseen_offset_counter;

							if (offset_pos[lastseen_offset] >= offset_pos[j]) {
								block_distance = offset_pos[j] + 4 - offset_pos[lastseen_offset];
							}
							else {
								block_distance = offset_pos[j] - offset_pos[lastseen_offset];
							}

							if ((block_distance * 26) != bit_distance) {
								presync = false;
							}
							else {
								//printf("@@@@@ Sync State Detected\n");
								enter_sync(j);
							}
						}

						break; //syndrome found, no more cycles
					}
				}

				break;
			}
			case ST_SYNC: {
				/* wait until 26 bits enter the buffer */
				if (block_bit_counter < 25) {
					block_bit_counter++;
				}
				else {
					good_block = false;
					dataword = (reg >> 10) & 0xffff;
					block_calculated_crc = calc_syndrome(dataword, 16);
					checkword = reg & 0x3ff;

					/* manage special case of C or C' offset word */
					if (block_number == 2) {
						block_received_crc = checkword ^ offset_word[2];

						if (block_received_crc == block_calculated_crc) {
							good_block = true;
						}
						else {
							block_received_crc = checkword ^ offset_word[4];

							if (block_received_crc == block_calculated_crc) {
								good_block = true;
							}
							else {
								wrong_blocks_counter++;
								good_block = false;
							}
						}
					}
					else {
						block_received_crc = checkword ^ offset_word[block_number];

						if (block_received_crc == block_calculated_crc) {
							good_block = true;
						}
						else {
							wrong_blocks_counter++;
							good_block = false;
						}
					}

					/* done checking CRC */
					if (block_number == 0 && good_block) {
						//std::cout << "Group assembly started!" << std::endl;
						group_assembly_started = true;
						group_good_blocks_counter = 1;
					}

					if (group_assembly_started) {
						if (!good_block) {
							group_assembly_started = false;
						}
						else {
							group[block_number] = dataword;
							group_good_blocks_counter++;
						}

						if (group_good_blocks_counter == 5) {
							decode_group(group);
						}
					}

					block_bit_counter = 0;
					block_number = (block_number + 1) % 4;
					blocks_counter++;
					/* 1187.5 bps / 104 bits = 11.4 groups/sec, or 45.7 blocks/sec */
					if (blocks_counter == 50) {
						if (wrong_blocks_counter > 35) {
							//printf("@@@@@ Lost Sync (Got %u bad blocks on %u total)\n", wrong_blocks_counter, blocks_counter);
							enter_no_sync();
						}
						else {
							//printf("@@@@@ Still Sync-ed (Got %u bad blocks on %u total)\n", wrong_blocks_counter, blocks_counter);
						}

						blocks_counter = 0;
						wrong_blocks_counter = 0;
					}
				}

				break;
			}
			default: {
				d_state = ST_NO_SYNC;
				break;
			}
		}

		i++;
		bit_counter++;
	}

	delete input;

	return NORMAL;
}

void RBDSDecoder_i::reset(void) {
	bit_counter = 0;
	reg = 0;
	reset_rds_data();
	enter_no_sync();
}

void RBDSDecoder_i::enter_no_sync() {
	presync=false;
	d_state = ST_NO_SYNC;
}

void RBDSDecoder_i::enter_sync(unsigned int sync_block_number) {
	wrong_blocks_counter = 0;
	blocks_counter = 0;
	block_bit_counter = 0;
	block_number = (sync_block_number + 1) % 4;
	group_assembly_started = false;
	d_state = ST_SYNC;
}

void RBDSDecoder_i::reset_rds_data() {
	memset(radiotext, ' ', sizeof(radiotext));
	radiotext[64] = '\0';
	radiotext_AB_flag = 0;
	traffic_program = false;
	traffic_announcement = false;
	music_speech = false;
	program_type = 0;
	memset(program_service_name, ' ', sizeof(program_service_name));
	program_service_name[8] = '\0';
	mono_stereo = false;
	artificial_head = false;
	compressed = false;
	static_pty = false;
}

unsigned int RBDSDecoder_i::calc_syndrome(unsigned long message, unsigned char mlen) {
	unsigned long reg = 0;
	unsigned int i;
	const unsigned long poly = 0x5B9;
	const unsigned char plen = 10;

	for (i = mlen; i > 0; i--) {
		reg = (reg << 1) | ((message >> (i - 1)) & 0x01);
		if (reg & (1 << plen))
			reg = reg ^ poly;
	}
	for (i = plen; i > 0; i--) {
		reg = reg << 1;
		if (reg & (1 << plen))
			reg = reg ^ poly;
	}

	return (reg & ((1 << plen) - 1)); // select the bottom plen bits of reg
}

void RBDSDecoder_i::decode_group(unsigned int* group) {
	unsigned int group_type = (unsigned int) ((group[1] >> 12) & 0xf);
	bool version_code = (group[1] >> 11) & 0x1;

	program_identification = group[0]; // "PI"
	program_type = (group[1] >> 5) & 0x1f; // "PTY"

	decode_callsign(program_identification);
	sprintf(pistring, "%04X", program_identification);

	/*printf("%02i%c ", group_type, (version_code ? 'B' : 'A'));
	std::cout << "(" << rds_group_acronyms[group_type] << ")";
	std::cout << "pistring: " << pistring << std::endl;
	std::cout << "program type: " << pty_table[program_type] << std::endl;
	std::cout << "Call Sign: " << callsign << std::endl; */

	switch (group_type) {
	case 0:
		decode_type0(group, version_code);
		break;
	case 1:
		break;
	case 2:
		decode_type2(group, version_code);
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		break;
	case 11:
		break;
	case 12:
		break;
	case 13:
		break;
	case 14:
		break;
	case 15:
		break;
	default:
		printf("DECODE ERROR!!! (group_type=%u)\n", group_type);
		break;
	}
}

void RBDSDecoder_i::decode_type0(unsigned int* group, bool version_code) {
	unsigned int af_code_1 = 0, af_code_2 = 0, no_af = 0;
	double af_1 = 0, af_2 = 0;
	char flagstring[8] = "0000000";

	traffic_program = (group[1] >> 10) & 0x01; // "TP"
	traffic_announcement = (group[1] >> 4) & 0x01; // "TA"
	music_speech = (group[1] >> 3) & 0x01; // "MuSp"
	bool decoder_control_bit = (group[1] >> 2) & 0x01; // "DI"
	unsigned char segment_address = group[1] & 0x03; // "DI segment"
	program_service_name[segment_address * 2] = (group[3] >> 8) & 0xff;
	program_service_name[segment_address * 2 + 1] = group[3] & 0xff;
	/* see page 41, table 9 of the standard */
	switch (segment_address) {
	case 0:
		mono_stereo = decoder_control_bit;
		break;
	case 1:
		artificial_head = decoder_control_bit;
		break;
	case 2:
		compressed = decoder_control_bit;
		break;
	case 3:
		static_pty = decoder_control_bit;
		break;
	default:
		break;
	}
	flagstring[0] = traffic_program ? '1' : '0';
	flagstring[1] = traffic_announcement ? '1' : '0';
	flagstring[2] = music_speech ? '1' : '0';
	flagstring[3] = mono_stereo ? '1' : '0';
	flagstring[4] = artificial_head ? '1' : '0';
	flagstring[5] = compressed ? '1' : '0';
	flagstring[6] = static_pty ? '1' : '0';
	if (!version_code) { // type 0A
		af_code_1 = (int) (group[2] >> 8) & 0xff;
		af_code_2 = (int) group[2] & 0xff;
		if ((af_1 = decode_af(af_code_1)))
			no_af += 1;
		if ((af_2 = decode_af(af_code_2)))
			no_af += 2;
		/* only AF1 => no_af==1, only AF2 => no_af==2, both AF1 and AF2 => no_af==3 */
		memset(af1_string, ' ', sizeof(af1_string));
		memset(af2_string, ' ', sizeof(af2_string));
		memset(af_string, ' ', sizeof(af_string));
		af1_string[9] = af2_string[9] = af_string[20] = '\0';
		if (no_af) {
			if (af_1 > 80e3)
				sprintf(af1_string, "%2.2fMHz", af_1 / 1e3);
			else if ((af_1 < 2e3) && (af_1 > 100))
				sprintf(af1_string, "%ikHz", (int) af_1);
			if (af_2 > 80e3)
				sprintf(af2_string, "%2.2fMHz", af_2 / 1e3);
			else if ((af_2 < 2e3) && (af_2 > 100))
				sprintf(af2_string, "%ikHz", (int) af_2);
		}
		if (no_af == 1)
			strcpy(af_string, af1_string);
		else if (no_af == 2)
			strcpy(af_string, af2_string);
		else if (no_af == 3) {
			strcpy(af_string, af1_string);
			strcat(af_string, ", ");
			strcat(af_string, af2_string);
		}
	}
	/*std::cout << "==>" << program_service_name << "<== -"
			<< (traffic_program ? "TP" : " ") << '-'
			<< (traffic_announcement ? "TA" : " ") << '-'
			<< (music_speech ? "Music" : "Speech") << '-'
			<< (mono_stereo ? "MONO" : "STEREO") << " - AF:" << af_string
			<< std::endl; */

	groupID[0] = '0';
	groupID[1] = version_code ? 'B' : 'A';

	RBDS_Output_struct mess;
	mess.Call_Sign = callsign;
	mess.PI_String = pistring;
	mess.Short_Text = program_service_name;
	mess.Full_Text = radiotext;
	mess.Station_Type = pty_table[program_type];
	mess.Group = groupID;
	mess.TextFlag = radiotext_flag;
	Message_Out->sendMessage(mess);

	LOG_INFO(RBDSDecoder_i, "\nmess.Call_Sign = " << mess.Call_Sign << "\nmess.PI_String = " << mess.PI_String
			<< "\nmess.Short_Text = " << mess.Short_Text << "\nmess.Full_Text = " << mess.Full_Text
			<< "\nmess.Station_Type = " << mess.Station_Type << "\nmess.Group = " << mess.Group
			<< "\nmess.TextFlag = " << mess.TextFlag << "\n");
}

void RBDSDecoder_i::decode_type2(unsigned int* group, bool version_code) {
	unsigned char text_segment_address_code = group[1] & 0x0f;

	/* when the A/B flag is toggled, flush your current radiotext */
	if (radiotext_AB_flag != ((group[1] >> 4) & 0x01)) {
		for (int i = 0; i < 64; i++)
			radiotext[i] = ' ';
		radiotext[64] = '\0';
	}
	radiotext_AB_flag = (group[1] >> 4) & 0x01;

	if (!version_code) {
		radiotext[text_segment_address_code * 4] = (group[2] >> 8) & 0xff;
		radiotext[text_segment_address_code * 4 + 1] = group[2] & 0xff;
		radiotext[text_segment_address_code * 4 + 2] = (group[3] >> 8) & 0xff;
		radiotext[text_segment_address_code * 4 + 3] = group[3] & 0xff;
	} else {
		radiotext[text_segment_address_code * 2] = (group[3] >> 8) & 0xff;
		radiotext[text_segment_address_code * 2 + 1] = group[3] & 0xff;
	}
	//printf("Radio Text %c: %s\n", (radiotext_AB_flag ? 'B' : 'A'), radiotext);
	//std::string flag = (radiotext_AB_flag ? 'B' : 'A');
	//std::cout << "Radio Text: " << radiotext << std::endl;
	//std::cout << std::endl;

	groupID[0] = '2';
	groupID[1] = version_code ? 'B' : 'A';
	radiotext_flag = radiotext_AB_flag ? 'B' : 'A';

	RBDS_Output_struct mess;
	mess.Call_Sign = callsign;
	mess.PI_String = pistring;
	mess.Short_Text = program_service_name;
	mess.Full_Text = radiotext;
	mess.Station_Type = pty_table[program_type];
	mess.Group = groupID;
	mess.TextFlag = radiotext_AB_flag ? 'B' : 'A';
	Message_Out->sendMessage(mess);

	LOG_INFO(RBDSDecoder_i, "\nmess.Call_Sign = " << mess.Call_Sign << "\nmess.PI_String = " << mess.PI_String
			<< "\nmess.Short_Text = " << mess.Short_Text << "\nmess.Full_Text = " << mess.Full_Text
			<< "\nmess.Station_Type = " << mess.Station_Type << "\nmess.Group = " << mess.Group
			<< "\nmess.TextFlag = " << mess.TextFlag << "\n");
}

double RBDSDecoder_i::decode_af(unsigned int af_code) {
	static unsigned int number_of_freqs;
	double alt_frequency = 0; // in kHz
	static bool vhf_or_lfmf = 0; // 0 = vhf, 1 = lf/mf

	/* in all the following cases the message either tells us
	 * that there are no alternative frequencies, or it indicates
	 * the number of AF to follow, which is not relevant at this
	 * stage, since we're not actually re-tuning */
	if ((af_code == 0) || // not to be used
			(af_code == 205) || // filler code
			((af_code >= 206) && (af_code <= 223)) || // not assigned
			(af_code == 224) || // No AF exists
			(af_code >= 251)) { // not assigned
		number_of_freqs = 0;
		alt_frequency = 0;
	}
	if ((af_code >= 225) && (af_code <= 249)) { // VHF frequencies follow
		number_of_freqs = af_code - 224;
		alt_frequency = 0;
		vhf_or_lfmf = 1;
	}
	if (af_code == 250) { // an LF/MF frequency follows
		number_of_freqs = 1;
		alt_frequency = 0;
		vhf_or_lfmf = 0;
	}

	/* here we're actually decoding the alternative frequency */
	if ((af_code > 0) && (af_code < 205) && vhf_or_lfmf)
		alt_frequency = (double) (af_code + 875) * 100; // VHF (87.6-107.9MHz)
	else if ((af_code > 0) && (af_code < 16) && !vhf_or_lfmf)
		alt_frequency = (double) ((af_code - 1) * 9 + 153); // LF (153-279kHz)
	else if ((af_code > 15) && (af_code < 136) && !vhf_or_lfmf)
		alt_frequency = (double) ((af_code - 16) * 9 + 531); // MF (531-1602kHz)

	return alt_frequency;
}

void RBDSDecoder_i::decode_callsign(unsigned int PI){
	unsigned int val1=0;
	unsigned int val2=0;
	unsigned int val3 = 0;

	if(PI > 4095 && PI < 21672){
		PI -= 4096;
		val1 = int(PI/676);
		PI -= (val1*676);

		val2 = int(PI/26);
		PI -= (val2*26);

		val3 = PI;

		callsign[0] = 'K';
		callsign[1] = call_letters[val1];
		callsign[2] = call_letters[val2];
		callsign[3] = call_letters[val3];
		callsign[4] = '\0';
	}

	if(PI > 21671 && PI < 39248){
		PI -= 21672;
		val1 = int(PI/676);
		PI -= (val1*676);

		val2 = int(PI/26);
		PI -= (val2*26);

		val3 = PI;

		callsign[0] = 'W';
		callsign[1] = call_letters[val1];
		callsign[2] = call_letters[val2];
		callsign[3] = call_letters[val3];
		callsign[4] = '\0';
	}

	if(PI > 39247 && PI < 40703){

		std::string temp = " ";
		temp.append(callMap[PI]);

		callsign[0] = temp[0];
		callsign[1] = temp[1];
		callsign[2] = temp[2];
		callsign[3] = temp[3];
		callsign[4] = '\0';
	}
}

void RBDSDecoder_i::set_call_map(){
	// 3 Letter Only Call Letters
	//See Page 88, Table D.4 in the standard
	callMap[0x99A5]="KBW";
	callMap[0x99A6]="KCY";
	callMap[0x9990]="KDB";
	callMap[0x99A7]="KDF";
	callMap[0x9950]="KEX";
	callMap[0x9951]="KFH";
	callMap[0x9952]="KFI";
	callMap[0x9953]="KGA";
	callMap[0x9991]="KGB";
	callMap[0x9954]="KGO";
	callMap[0x9955]="KGU";
	callMap[0x9956]="KGW";
	callMap[0x9957]="KGY";
	callMap[0x99AA]="KHQ";
	callMap[0x9958]="KID";
	callMap[0x9959]="KIT";
	callMap[0x995A]="KJR";
	callMap[0x995B]="KLO";
	callMap[0x995C]="KLZ";
	callMap[0x995D]="KMA";
	callMap[0x995E]="KMJ";
	callMap[0x995F]="KNX";
	callMap[0x9960]="KOA";
	callMap[0x99AB]="KOB";

	callMap[0x9992]="KOY";
	callMap[0x9993]="KPQ";
	callMap[0x9964]="KQV";
	callMap[0x9994]="KSD";
	callMap[0x9965]="KSL";
	callMap[0x9966]="KUJ";
	callMap[0x9995]="KUT";
	callMap[0x9967]="KVI";
	callMap[0x9968]="KWG";
	callMap[0x9996]="KXL";
	callMap[0x9997]="KXO";
	callMap[0x996B]="KYW";
	callMap[0x9999]="WBT";
	callMap[0x996D]="WBZ";
	callMap[0x996E]="WDZ";
	callMap[0x996F]="WEW";
	callMap[0x999A]="WGH";
	callMap[0x9971]="WGL";
	callMap[0x9972]="WGN";
	callMap[0x9973]="WGR";
	callMap[0x999B]="WGY";
	callMap[0x9975]="WHA";
	callMap[0x9976]="WHB";
	callMap[0x9977]="WHK";

	callMap[0x9978]="WHO";
	callMap[0x999C]="WHP";
	callMap[0x999D]="WIL";
	callMap[0x997A]="WIP";
	callMap[0x99B3]="WIS";
	callMap[0x997B]="WJR";
	callMap[0x99B4]="WJW";
	callMap[0x99B5]="WJZ";
	callMap[0x997C]="WKY";
	callMap[0x997D]="WLS";
	callMap[0x997E]="WLW";
	callMap[0x999E]="WMC";
	callMap[0x999F]="WMT";
	callMap[0x9981]="WOC";
	callMap[0x99A0]="WOI";
	callMap[0x9983]="WOL";
	callMap[0x9984]="WOR";
	callMap[0x99A1]="WOW";
	callMap[0x99B9]="WRC";
	callMap[0x99A2]="WRR";
	callMap[0x99A3]="WSB";
	callMap[0x99A4]="WSM";
	callMap[0x9988]="WWJ";
	callMap[0x9989]="WWL";
}
