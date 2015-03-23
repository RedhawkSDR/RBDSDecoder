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
#ifndef INCLUDED_RDS_DECODER_IMPL_H
#define INCLUDED_RDS_DECODER_IMPL_H
#include <vector>
#include "parser_impl.h"
#include "../LoggingInterface.h"
#include "../EventingInterface.h"

class decoder_impl {
public:
	decoder_impl(GenericInterface::LoggingInterface *logger, GenericInterface::EventingInterface * eventServer);
	~decoder_impl();

	void reset();
	void work(std::vector<short> &in);

private:

	GenericInterface::LoggingInterface *logger;
	GenericInterface::EventingInterface *eventServer;
	void enter_no_sync();
	void enter_sync(unsigned int);
	unsigned int calc_syndrome(unsigned long, unsigned char);
	void decode_group(unsigned int*);


	unsigned long  bit_counter;
	unsigned long  lastseen_offset_counter, reg;
	unsigned int   block_bit_counter;
	unsigned int   wrong_blocks_counter;
	unsigned int   blocks_counter;
	unsigned int   group_good_blocks_counter;
	unsigned int   group[4];
	bool           debug;
	bool           log;
	bool           presync;
	bool           good_block;
	bool           group_assembly_started;
	unsigned char  lastseen_offset;
	unsigned char  block_number;
	enum { NO_SYNC, SYNC } d_state;
	parser_impl parser;

};

#endif /* INCLUDED_RDS_DECODER_IMPL_H */

