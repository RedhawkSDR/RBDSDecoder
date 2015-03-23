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

#ifndef PORT_H
#define PORT_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

class RBDSDecoder_base;
class RBDSDecoder_i;

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#include <ossie/MessageInterface.h>
#include "struct_props.h"

// ----------------------------------------------------------------------------------------
// ExtendedEvent_MessageEvent_Out_i declaration
// ----------------------------------------------------------------------------------------
class ExtendedEvent_MessageEvent_Out_i : public MessageSupplierPort
{
    public:
        ExtendedEvent_MessageEvent_Out_i(std::string port_name) : MessageSupplierPort(port_name) {
        };

        void sendMessage(RBDS_Output_struct message) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(1);
            outProps[0].id = CORBA::string_dup(message.getId().c_str());
            outProps[0].value <<= message;
            data <<= outProps;
            push(data);
        }

        void sendMessages(std::vector<RBDS_Output_struct> messages) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(messages.size());
            for (unsigned int i=0; i<messages.size(); i++) {
                outProps[i].id = CORBA::string_dup(messages[i].getId().c_str());
                outProps[i].value <<= messages[i];
            }
            data <<= outProps;
            push(data);
        }
};

#endif
