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

#ifndef LOGGINGINTERFACE_H_
#define LOGGINGINTERFACE_H_

namespace GenericInterface {

#define LOG( level, msg )	\
{ \
  std::ostringstream _msg;						\
  _msg <<  msg;				          		\
  logger->log( level, _msg.str());	\
} \


class LoggingInterface {
public:
	enum LEVEL {TRACE, INFO, DEBUG, WARN, ERROR, FATAL};
	virtual void log(enum LEVEL level, std::string msg) = 0;
};

}

#endif /* LOGGINGINTERFACE_H_ */
