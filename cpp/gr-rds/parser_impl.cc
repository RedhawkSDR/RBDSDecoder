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

#include "parser_impl.h"
#include "constants.h"
#include "tmc_events.h"
#include <math.h>
#include <string>
#include <cstring>
#include "boost/format.hpp"
#include <iomanip>

parser_impl::parser_impl(GenericInterface::LoggingInterface *logger, GenericInterface::EventingInterface * eventServer) :
showType3BNotImplWarning(true), showType4BNotImplWarning(true), showType5NotImplWarning(true), showType6NotImplWarning(true), showType7NotImplWarning(true), showType8BNotImplWarning(true), showType9NotImplWarning(true), showType10NotImplWarning(true), showType11NotImplWarning(true), showType12NotImplWarning(true), showType13NotImplWarning(true), showType15NotImplWarning(true)
{
	this->logger = logger;
	this->eventServer = eventServer;
	reset();
}

parser_impl::~parser_impl() {
}

void parser_impl::reset() {
	memset(radiotext, ' ', sizeof(radiotext));
	memset(program_service_name, '.', sizeof(program_service_name));

	radiotext_AB_flag              = 0;
	traffic_program                = false;
	traffic_announcement           = false;
	music_speech                   = false;
	program_type                   = 0;
	pi_country_identification      = 0;
	pi_area_coverage               = 0;
	pi_program_reference_number    = 0;
	mono_stereo                    = false;
	artificial_head                = false;
	compressed                     = false;
	static_pty                     = false;
}

/* BASIC TUNING: see page 21 of the standard */
void parser_impl::decode_type0(unsigned int *group, bool B) {
	unsigned int af_code_1 = 0;
	unsigned int af_code_2 = 0;
	unsigned int  no_af    = 0;
	double af_1            = 0;
	double af_2            = 0;
	char flagstring[8]     = "0000000";
	
	traffic_program        = (group[1] >> 10) & 0x01;       // "TP"
	traffic_announcement   = (group[1] >>  4) & 0x01;       // "TA"
	music_speech           = (group[1] >>  3) & 0x01;       // "MuSp"

	bool decoder_control_bit      = (group[1] >> 2) & 0x01; // "DI"
	unsigned char segment_address =  group[1] & 0x03;       // "DI segment"

	program_service_name[segment_address * 2]     = (group[3] >> 8) & 0xff;
	program_service_name[segment_address * 2 + 1] =  group[3]       & 0xff;

	/* see page 41, table 9 of the standard */
	switch (segment_address) {
		case 0:
			mono_stereo=decoder_control_bit;
		break;
		case 1:
			artificial_head=decoder_control_bit;
		break;
		case 2:
			compressed=decoder_control_bit;
		break;
		case 3:
			static_pty=decoder_control_bit;
		break;
		default:
		break;
	}
	std::stringstream flagstring_ss;
	flagstring_ss << (traffic_program ? "traffic_program:" : "");
	flagstring[0] = traffic_program        ? '1' : '0';

	flagstring_ss << (traffic_announcement ? ", traffic_announcement" : "");
	flagstring[1] = traffic_announcement   ? '1' : '0';

	flagstring_ss << (music_speech ? "music_speech:" : "");
	flagstring[2] = music_speech           ? '1' : '0';

	flagstring_ss << (mono_stereo ? "mono_stereo:" : "");
	flagstring[3] = mono_stereo            ? '1' : '0';

	flagstring_ss << (artificial_head ? "artificial_head:" : "");
	flagstring[4] = artificial_head        ? '1' : '0';

	flagstring_ss << (compressed ? "compressed:" : "");
	flagstring[5] = compressed             ? '1' : '0';

	flagstring_ss << (static_pty ? "static_pty:" : "");
	flagstring[6] = static_pty             ? '1' : '0';
	static std::string af_string;

	if(!B) { // type 0A
		af_code_1 = int(group[2] >> 8) & 0xff;
		af_code_2 = int(group[2])      & 0xff;
		af_1 = decode_af(af_code_1);
		af_2 = decode_af(af_code_2);

		if(af_1) {
			no_af += 1;
		}
		if(af_2) {
			no_af += 2;
		}

		std::string af1_string;
		std::string af2_string;
		/* only AF1 => no_af==1, only AF2 => no_af==2, both AF1 and AF2 => no_af==3 */
		if(no_af) {
			if(af_1 > 80e3) {
				af1_string = str(boost::format("%2.2fMHz") % (af_1/1e3));
			} else if((af_1<2e3)&&(af_1>100)) {
				af1_string = str(boost::format("%ikHz") % int(af_1));
			}
			if(af_2 > 80e3) {
				af2_string = str(boost::format("%2.2fMHz") % (af_2/1e3));
			} else if ((af_2 < 2e3) && (af_2 > 100)) {
				af2_string = str(boost::format("%ikHz") % int(af_2));
			}
		}
		if(no_af == 1) {
			af_string = af1_string;
		} else if(no_af == 2) {
			af_string = af2_string;
		} else if(no_af == 3) {
			af_string = str(boost::format("%s, %s") % af1_string %af2_string);
		}
	}
	LOG(LoggingInterface::TRACE, std::string(program_service_name, 8)
		<< "<== -" << (traffic_program ? "TP" : "  ")
		<< '-' << (traffic_announcement ? "TA" : "  ")
		<< '-' << (music_speech ? "Music" : "Speech")
		<< '-' << (mono_stereo ? "MONO" : "STEREO")
		<< " - AF:" << af_string);

	eventServer->sendMessage(EventingInterface::PS, B ? "00B" : "00A", std::string(program_service_name, 8));
	eventServer->sendMessage(EventingInterface::FLAG_STRING, B ? "00B" : "00A", flagstring_ss.str());
	eventServer->sendMessage(EventingInterface::ALT_FREQ, B ? "00B" : "00A", af_string);
}

double parser_impl::decode_af(unsigned int af_code) {
	static unsigned int number_of_freqs = 0;
	static bool vhf_or_lfmf             = 0; // 0 = vhf, 1 = lf/mf
	double alt_frequency                = 0; // in kHz

	if((af_code == 0) ||                              // not to be used
		( af_code == 205) ||                      // filler code
		((af_code >= 206) && (af_code <= 223)) || // not assigned
		( af_code == 224) ||                      // No AF exists
		( af_code >= 251)) {                      // not assigned
			number_of_freqs = 0;
			alt_frequency   = 0;
	}
	if((af_code >= 225) && (af_code <= 249)) {        // VHF frequencies follow
		number_of_freqs = af_code - 224;
		alt_frequency   = 0;
		vhf_or_lfmf     = 1;
	}
	if(af_code == 250) {                              // an LF/MF frequency follows
		number_of_freqs = 1;
		alt_frequency   = 0;
		vhf_or_lfmf     = 0;
	}

	if((af_code > 0) && (af_code < 205) && vhf_or_lfmf)
		alt_frequency = 100.0 * (af_code + 875);          // VHF (87.6-107.9MHz)
	else if((af_code > 0) && (af_code < 16) && !vhf_or_lfmf)
		alt_frequency = 153.0 + (af_code - 1) * 9;        // LF (153-279kHz)
	else if((af_code > 15) && (af_code < 136) && !vhf_or_lfmf)
		alt_frequency = 531.0 + (af_code - 16) * 9 + 531; // MF (531-1602kHz)

	return alt_frequency;
}

void parser_impl::decode_type1(unsigned int *group, bool B){
	int ecc    = 0;
	int paging = 0;
	char country_code           = (group[0] >> 12) & 0x0f;
	char radio_paging_codes     =  group[1]        & 0x1f;
	int variant_code            = (group[2] >> 12) & 0x7;
	unsigned int slow_labelling =  group[2]        & 0xfff;
	int day    = (int)((group[3] >> 11) & 0x1f);
	int hour   = (int)((group[3] >>  6) & 0x1f);
	int minute = (int) (group[3]        & 0x3f);

	if(radio_paging_codes) {
		LOG(LoggingInterface::TRACE, "paging codes: " << int(radio_paging_codes));
	}

	if(day || hour || minute) {
		LOG(LoggingInterface::TRACE, boost::format("program item: %id, %i, %i ") % day % hour % minute);
	}

	if(!B){
		switch(variant_code){
			case 0: // paging + ecc
				paging = (slow_labelling >> 8) & 0x0f;
				ecc    =  slow_labelling       & 0xff;
				if(paging) {
					LOG(LoggingInterface::TRACE, "paging: " << paging);
				}
				if((ecc > 223) && (ecc < 229)) {
					LOG(LoggingInterface::TRACE, "extended country code: " << pi_country_codes[country_code-1][ecc-224]);
				} else {
					LOG(LoggingInterface::WARN, "invalid extended country code: " << ecc);
				}
				break;
			case 1: // TMC identification
				LOG(LoggingInterface::INFO, "TMC identification code received");
				break;
			case 2: // Paging identification
				LOG(LoggingInterface::TRACE, "Paging identification code received");
				break;
			case 3: // language codes
				if(slow_labelling < 44) {
					LOG(LoggingInterface::TRACE, "language: " << language_codes[slow_labelling]);
				} else {
					LOG(LoggingInterface::TRACE, "language: invalid language code " << slow_labelling);
				}
				break;
			default:
				break;
		}
	}
}

void parser_impl::decode_type2(unsigned int *group, bool B){
	unsigned char text_segment_address_code = group[1] & 0x0f;

	// when the A/B flag is toggled, flush your current radiotext
	if(radiotext_AB_flag != ((group[1] >> 4) & 0x01)) {
		std::memset(radiotext, ' ', sizeof(radiotext));
	}
	radiotext_AB_flag = (group[1] >> 4) & 0x01;

	if(!B) {
		radiotext[text_segment_address_code *4     ] = (group[2] >> 8) & 0xff;
		radiotext[text_segment_address_code * 4 + 1] =  group[2]       & 0xff;
		radiotext[text_segment_address_code * 4 + 2] = (group[3] >> 8) & 0xff;
		radiotext[text_segment_address_code * 4 + 3] =  group[3]       & 0xff;
	} else {
		radiotext[text_segment_address_code * 2    ] = (group[3] >> 8) & 0xff;
		radiotext[text_segment_address_code * 2 + 1] =  group[3]       & 0xff;
	}
	LOG(LoggingInterface::TRACE, "Radio Text " << (radiotext_AB_flag ? 'B' : 'A')
		<< ": " << std::string(radiotext, sizeof(radiotext)));

	eventServer->sendMessage(EventingInterface::RADIO_TEXT, B ? "02B" : "02A", std::string(radiotext, sizeof(radiotext)));
}

void parser_impl::decode_type3(unsigned int *group, bool B){
	if(B) {
		notImplementedWarning("3B", showType3BNotImplWarning);
		return;
	}

	int application_group = (group[1] >> 1) & 0xf;
	int group_type        =  group[1] & 0x1;
	int message           =  group[2];
	int aid               =  group[3];
	
	LOG(LoggingInterface::TRACE, "aid group: " << application_group
		<< " " << (group_type ? 'B' : 'A'));

	if((application_group == 8) && (group_type == false)) { // 8A
		int variant_code = (message >> 14) & 0x3;
		if(variant_code == 0) {
			int ltn  = (message >> 6) & 0x3f; // location table number
			bool afi = (message >> 5) & 0x1;  // alternative freq. indicator
			bool M   = (message >> 4) & 0x1;  // mode of transmission
			bool I   = (message >> 3) & 0x1;  // international
			bool N   = (message >> 2) & 0x1;  // national
			bool R   = (message >> 1) & 0x1;  // regional
			bool U   =  message       & 0x1;  // urban

			LOG(LoggingInterface::TRACE, "location table: " << ltn << " - "
				<< (afi ? "AFI-ON" : "AFI-OFF") << " - "
				<< (M   ? "enhanced mode" : "basic mode") << " - "
				<< (I   ? "international " : "")
				<< (N   ? "national " : "")
				<< (R   ? "regional " : "")
				<< (U   ? "urban" : "")
				<< " aid: " << aid);

		} else if(variant_code==1) {
			int G   = (message >> 12) & 0x3;  // gap
			int sid = (message >>  6) & 0x3f; // service identifier
			int gap_no[4] = {3, 5, 8, 11};
			LOG(LoggingInterface::TRACE, "gap: " << gap_no[G] << " groups, SID: " << sid << " ");
		}
	}
	LOG(LoggingInterface::TRACE, "message: " << message << " - aid: " << aid << std::endl);
}

void parser_impl::decode_type4(unsigned int *group, bool B){
	if(B) {
		notImplementedWarning("4B", showType4BNotImplWarning);
		return;
	}

	unsigned int hours   = ((group[2] & 0x1) << 4) | ((group[3] >> 12) & 0x0f);
	unsigned int minutes =  (group[3] >> 6) & 0x3f;
	double local_time_offset = .5 * (group[3] & 0x1f);

	if((group[3] >> 5) & 0x1) {
		local_time_offset *= -1;
	}
	double modified_julian_date = ((group[1] & 0x03) << 15) | ((group[2] >> 1) & 0x7fff);

	unsigned int year  = int((modified_julian_date - 15078.2) / 365.25);
	unsigned int month = int((modified_julian_date - 14956.1 - int(year * 365.25)) / 30.6001);
	unsigned int day   =               modified_julian_date - 14956 - int(year * 365.25) - int(month * 30.6001);
	bool K = ((month == 14) || (month == 15)) ? 1 : 0;
	year += K;
	month -= 1 + K * 12;

	std::string time = str(boost::format("%02i.%02i.%4i, %02i:%02i (%+.1fh)")\
		% day % month % (1900 + year) % hours % minutes % local_time_offset);
	LOG(LoggingInterface::TRACE, "Clocktime: " << time);

	eventServer->sendMessage(EventingInterface::CLOCK_TIME, B ? "04B" : "04A", time);
}

void parser_impl::decode_type5(unsigned int *group, bool B){
	notImplementedWarning("5", showType5NotImplWarning);
}

void parser_impl::decode_type6(unsigned int *group, bool B){
	notImplementedWarning("6", showType6NotImplWarning);
}

void parser_impl::decode_type7(unsigned int *group, bool B){
	notImplementedWarning("7", showType7NotImplWarning);
}

void parser_impl::decode_type8(unsigned int *group, bool B){
	if(B) {
		notImplementedWarning("8B", showType8BNotImplWarning);
		return;
	}
	bool T = (group[1] >> 4) & 0x1; // 0 = user message, 1 = tuning info
	bool F = (group[1] >> 3) & 0x1; // 0 = multi-group, 1 = single-group
	bool D = (group[2] > 15) & 0x1; // 1 = diversion recommended
	static unsigned long int free_format[4];
	static int no_groups = 0;

	if(T) { // tuning info
		LOG(LoggingInterface::TRACE, "#tuning info# ");
		int variant = group[1] & 0xf;
		if((variant > 3) && (variant < 10)) {
			LOG(LoggingInterface::TRACE, "variant: " << variant << " - "
				<< group[2] << " " << group[3]);
		} else {
			LOG(LoggingInterface::WARN, "invalid variant: " << variant);
		}

	} else if(F || D) { // single-group or 1st of multi-group
		unsigned int dp_ci    =  group[1]        & 0x7;   // duration & persistence or continuity index
		bool sign             = (group[2] >> 14) & 0x1;   // event direction, 0 = +, 1 = -
		unsigned int extent   = (group[2] >> 11) & 0x7;   // number of segments affected
		unsigned int event    =  group[2]        & 0x7ff; // event code, defined in ISO 14819-2
		unsigned int location =  group[3];                // location code, defined in ISO 14819-3
		LOG(LoggingInterface::TRACE, "#user msg# " << (D ? "diversion recommended, " : ""));
		if(F) {
			LOG(LoggingInterface::TRACE, "single-grp, duration:" << tmc_duration[dp_ci][0]);
		} else {
			LOG(LoggingInterface::TRACE, "multi-grp, continuity index:" << dp_ci);
		}
		int event_line = tmc_event_code_index[event][1];
		LOG(LoggingInterface::TRACE, ", extent:" << (sign ? "-" : "") << extent + 1 << " segments"
			<< ", event" << event << ":" << tmc_events[event_line][1]
			<< ", location:" << location);

	} else { // 2nd or more of multi-group
		unsigned int ci = group[1] & 0x7;          // countinuity index
		bool sg = (group[2] >> 14) & 0x1;          // second group
		unsigned int gsi = (group[2] >> 12) & 0x3; // group sequence
		LOG(LoggingInterface::TRACE, "#user msg# multi-grp, continuity index:" << ci
			<< (sg ? ", second group" : "") << ", gsi:" << gsi);
		LOG(LoggingInterface::TRACE, ", free format: " << (group[2] & 0xfff) << " "<< group[3]);
		// it's not clear if gsi=N-2 when gs=true
		if(sg) {
			no_groups = gsi;
		}
		free_format[gsi] = ((group[2] & 0xfff) << 12) | group[3];
		if(gsi == 0) {
			decode_optional_content(no_groups, free_format);
		}
	}
}

void parser_impl::decode_optional_content(int no_groups, unsigned long int *free_format){
	int label          = 0;
	int content        = 0;
	int content_length = 0;
	int ff_pointer     = 0;
	
	for (int i = no_groups; i == 0; i--){
		ff_pointer = 12 + 16;
		while(ff_pointer > 0){
			ff_pointer -= 4;
			label = (free_format[i] && (0xf << ff_pointer));
			content_length = optional_content_lengths[label];
			ff_pointer -= content_length;
			content = (free_format[i] && (int(pow(2, content_length) - 1) << ff_pointer));
			LOG(LoggingInterface::TRACE, "TMC optional content (" << label_descriptions[label] << "):" << content);
		}
	}
}

void parser_impl::decode_type9(unsigned int *group, bool B){
	notImplementedWarning("9", showType9NotImplWarning);
}

void parser_impl::decode_type10(unsigned int *group, bool B){
	notImplementedWarning("10", showType10NotImplWarning);
}

void parser_impl::decode_type11(unsigned int *group, bool B){
	notImplementedWarning("11", showType11NotImplWarning);
}

void parser_impl::decode_type12(unsigned int *group, bool B){
	notImplementedWarning("12", showType12NotImplWarning);
}

void parser_impl::decode_type13(unsigned int *group, bool B){
	notImplementedWarning("13", showType13NotImplWarning);
}

void parser_impl::decode_type14(unsigned int *group, bool B){
	
	bool tp_on               = (group[1] >> 4) & 0x01;
	char variant_code        = group[1] & 0x0f;
	unsigned int information = group[2];
	unsigned int pi_on       = group[3];
	
	char pty_on = 0;
	bool ta_on = 0;
	static char ps_on[8] = {' ',' ',' ',' ',' ',' ',' ',' '};
	double af_1 = 0;
	double af_2 = 0;
	
	if (!B){
		switch (variant_code){
			case 0: // PS(ON)
			case 1: // PS(ON)
			case 2: // PS(ON)
			case 3: // PS(ON)
				ps_on[variant_code * 2    ] = (information >> 8) & 0xff;
				ps_on[variant_code * 2 + 1] =  information       & 0xff;
				LOG(LoggingInterface::TRACE, "PS(ON): ==>" << std::string(ps_on, 8) << "<==");
			break;
			case 4: // AF
				af_1 = 100.0 * (((information >> 8) & 0xff) + 875);
				af_2 = 100.0 * ((information & 0xff) + 875);
				LOG(LoggingInterface::TRACE, boost::format("AF:%3.2fMHz %3.2fMHz") % (af_1/1000) % (af_2/1000));
			break;
			case 5: // mapped frequencies
			case 6: // mapped frequencies
			case 7: // mapped frequencies
			case 8: // mapped frequencies
				af_1 = 100.0 * (((information >> 8) & 0xff) + 875);
				af_2 = 100.0 * ((information & 0xff) + 875);
				LOG(LoggingInterface::TRACE, boost::format("TN:%3.2fMHz - ON:%3.2fMHz") % (af_1/1000) % (af_2/1000));
			break;
			case 9: // mapped frequencies (AM)
				af_1 = 100.0 * (((information >> 8) & 0xff) + 875);
				af_2 = 9.0 * ((information & 0xff) - 16) + 531;
				LOG(LoggingInterface::TRACE, boost::format("TN:%3.2fMHz - ON:%ikHz") % (af_1/1000) % int(af_2));
			break;
			case 10: // unallocated
			break;
			case 11: // unallocated
			break;
			case 12: // linkage information
				LOG(LoggingInterface::TRACE, boost::format("Linkage information: %x%x")
					% ((information >> 8) & 0xff) % (information & 0xff));
			break;
			case 13: // PTY(ON), TA(ON)
				ta_on = information & 0x01;
				pty_on = (information >> 11) & 0x1f;
				LOG(LoggingInterface::TRACE, "PTY(ON):" << pty_table[int(pty_on)]);
				if(ta_on) {
					LOG(LoggingInterface::TRACE, " - TA");
				}
			break;
			case 14: // PIN(ON)
				LOG(LoggingInterface::TRACE, boost::format("PIN(ON):%x%x")	% ((information >> 8) & 0xff) % (information & 0xff));
			break;
			case 15: // Reserved for broadcasters use
			break;
			default:
				LOG(LoggingInterface::TRACE, "invalid variant code:" << variant_code);
			break;
		}
	}
	if (pi_on){
		LOG(LoggingInterface::TRACE, " PI(ON):" << pi_on);
		if (tp_on) {
			LOG(LoggingInterface::TRACE, "-TP-");
		}
	}
}

void parser_impl::decode_type15(unsigned int *group, bool B){
	notImplementedWarning("15", showType15NotImplWarning);
}

void parser_impl::parse(unsigned int * group) {

	unsigned int group_type = (unsigned int)((group[1] >> 12) & 0xf);
	bool ab = (group[1] >> 11 ) & 0x1;

	std::stringstream type_ss;
	type_ss << boost::format("%02i%c ") % group_type % (ab ? 'B' :'A');
	std::string type_string = type_ss.str();

	LOG(LoggingInterface::TRACE, type_string);
	LOG(LoggingInterface::TRACE, "(" << rds_group_acronyms[group_type] << ")");

	program_identification = group[0];     // "PI"
	program_type = (group[1] >> 5) & 0x1f; // "PTY"
	int pi_country_identification = (program_identification >> 12) & 0xf;
	int pi_area_coverage = (program_identification >> 8) & 0xf;
	unsigned char pi_program_reference_number = program_identification & 0xff;
	std::stringstream pi_ss;
	pi_ss << program_identification;

	std::stringstream pty_ss;
	pty_ss << (unsigned int) program_type;

	LOG(LoggingInterface::TRACE, "program_identification: " << program_identification);
	eventServer->sendMessage(EventingInterface::PI, type_string, pi_ss.str());
	eventServer->sendMessage(EventingInterface::PTY, type_string, pty_ss.str());

	LOG(LoggingInterface::TRACE, " - PI:" << pi_ss.str() << " - " << "PTY:" << program_type
	<< " (country:" << pi_country_codes[pi_country_identification - 1][0]
	<< "/" << pi_country_codes[pi_country_identification - 1][1]
	<< "/" << pi_country_codes[pi_country_identification - 1][2]
	<< "/" << pi_country_codes[pi_country_identification - 1][3]
	<< "/" << pi_country_codes[pi_country_identification - 1][4]
	<< ", area:" << coverage_area_codes[pi_area_coverage]
	<< ", program:" << int(pi_program_reference_number) << ")");

	switch (group_type) {
		case 0:
			decode_type0(group, ab);
			break;
		case 1:
			decode_type1(group, ab);
			break;
		case 2:
			decode_type2(group, ab);
			break;
		case 3:
			decode_type3(group, ab);
			break;
		case 4:
			decode_type4(group, ab);
			break;
		case 5:
			decode_type5(group, ab);
			break;
		case 6:
			decode_type6(group, ab);
			break;
		case 7:
			decode_type7(group, ab);
			break;
		case 8:
			decode_type8(group, ab);
			break;
		case 9:
			decode_type9(group, ab);
			break;
		case 10:
			decode_type10(group, ab);
			break;
		case 11:
			decode_type11(group, ab);
			break;
		case 12:
			decode_type12(group, ab);
			break;
		case 13:
			decode_type13(group, ab);
			break;
		case 14:
			decode_type14(group, ab);
			break;
		case 15:
			decode_type15(group, ab);
			break;
	}

	#define HEX(a) std::hex << std::setfill('0') << std::setw(4) << long(a) << std::dec
	std::ostringstream hexMsg;
	for(int i = 0; i < 4; i++) {
		hexMsg << "  " << HEX(group[i]);
	}

	LOG(LoggingInterface::TRACE, hexMsg);
}

void parser_impl::notImplementedWarning(std::string type, bool notImplementedBoolean) {
	if (notImplementedBoolean) {
		LOG(LoggingInterface::WARN, "type " + type + " not implemented yet. This warning will only be displayed once.");
		notImplementedBoolean = false;
	}
	LOG(LoggingInterface::TRACE, "type " + type + " not implemented yet.");
}
