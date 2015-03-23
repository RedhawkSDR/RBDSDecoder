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
#ifndef INCLUDED_RDS_PARSER_IMPL_H
#define INCLUDED_RDS_PARSER_IMPL_H

#include <boost/thread/mutex.hpp>
#include <vector>
#include "../EventingInterface.h"
#include "../LoggingInterface.h"

using namespace GenericInterface;

class parser_impl {
public:
	parser_impl(GenericInterface::LoggingInterface *logger, GenericInterface::EventingInterface * eventServer);
	~parser_impl();
	void parse(unsigned int * group);

private:
	LoggingInterface *logger;
	EventingInterface * eventServer;
	void reset();
	double decode_af(unsigned int);
	void decode_optional_content(int, unsigned long int *);

	void decode_type0( unsigned int* group, bool B);
	void decode_type1( unsigned int* group, bool B);
	void decode_type2( unsigned int* group, bool B);
	void decode_type3( unsigned int* group, bool B);
	void decode_type4( unsigned int* group, bool B);
	void decode_type5( unsigned int* group, bool B);
	void decode_type6( unsigned int* group, bool B);
	void decode_type7( unsigned int* group, bool B);
	void decode_type8( unsigned int* group, bool B);
	void decode_type9( unsigned int* group, bool B);
	void decode_type10(unsigned int* group, bool B);
	void decode_type11(unsigned int* group, bool B);
	void decode_type12(unsigned int* group, bool B);
	void decode_type13(unsigned int* group, bool B);
	void decode_type14(unsigned int* group, bool B);
	void decode_type15(unsigned int* group, bool B);

	unsigned int   program_identification;
	unsigned char  program_type;
	unsigned char  pi_country_identification;
	unsigned char  pi_area_coverage;
	unsigned char  pi_program_reference_number;
	char           radiotext[65];
	char           program_service_name[9];
	bool           radiotext_AB_flag;
	bool           traffic_program;
	bool           traffic_announcement;
	bool           music_speech;
	bool           mono_stereo;
	bool           artificial_head;
	bool           compressed;
	bool           static_pty;
	bool           debug;
	bool           log;
	boost::mutex d_mutex;
};

#endif /* INCLUDED_RDS_PARSER_IMPL_H */

