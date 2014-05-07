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

#ifndef RBDSDECODER_IMPL_BASE_H
#define RBDSDECODER_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>
#include <ossie/MessageInterface.h>
#include "struct_props.h"

class RBDSDecoder_base : public Resource_impl, protected ThreadedComponent
{
    public:
        RBDSDecoder_base(const char *uuid, const char *label);
        ~RBDSDecoder_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        RBDS_Output_struct RBDS_Output;

        // Ports
        bulkio::InShortPort *dataIn;
        MessageSupplierPort *Message_Out;

    private:
};
#endif // RBDSDECODER_IMPL_BASE_H
