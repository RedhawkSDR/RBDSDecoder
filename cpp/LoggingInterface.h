/*
 * LoggingInterface.h
 *
 *  Created on: Mar 20, 2015
 *      Author: ylbagou
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
