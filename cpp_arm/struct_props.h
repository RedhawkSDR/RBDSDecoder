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

#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <ossie/PropertyInterface.h>

struct RBDS_Output_struct {
    RBDS_Output_struct ()
    {
    };

    static std::string getId() {
        return std::string("RBDS_Output");
    };

    std::string Full_Text;
    std::string Short_Text;
    std::string Station_Type;
    std::string Call_Sign;
    std::string PI_String;
    std::string Group;
    std::string TextFlag;
};

inline bool operator>>= (const CORBA::Any& a, RBDS_Output_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("Full_Text", props[idx].id)) {
            if (!(props[idx].value >>= s.Full_Text)) return false;
        }
        if (!strcmp("Short_Text", props[idx].id)) {
            if (!(props[idx].value >>= s.Short_Text)) return false;
        }
        if (!strcmp("Station_Type", props[idx].id)) {
            if (!(props[idx].value >>= s.Station_Type)) return false;
        }
        if (!strcmp("Call_Sign", props[idx].id)) {
            if (!(props[idx].value >>= s.Call_Sign)) return false;
        }
        if (!strcmp("PI_String", props[idx].id)) {
            if (!(props[idx].value >>= s.PI_String)) return false;
        }
        if (!strcmp("Group", props[idx].id)) {
            if (!(props[idx].value >>= s.Group)) return false;
        }
        if (!strcmp("TextFlag", props[idx].id)) {
            if (!(props[idx].value >>= s.TextFlag)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const RBDS_Output_struct& s) {
    CF::Properties props;
    props.length(7);
    props[0].id = CORBA::string_dup("Full_Text");
    props[0].value <<= s.Full_Text;
    props[1].id = CORBA::string_dup("Short_Text");
    props[1].value <<= s.Short_Text;
    props[2].id = CORBA::string_dup("Station_Type");
    props[2].value <<= s.Station_Type;
    props[3].id = CORBA::string_dup("Call_Sign");
    props[3].value <<= s.Call_Sign;
    props[4].id = CORBA::string_dup("PI_String");
    props[4].value <<= s.PI_String;
    props[5].id = CORBA::string_dup("Group");
    props[5].value <<= s.Group;
    props[6].id = CORBA::string_dup("TextFlag");
    props[6].value <<= s.TextFlag;
    a <<= props;
};

inline bool operator== (const RBDS_Output_struct& s1, const RBDS_Output_struct& s2) {
    if (s1.Full_Text!=s2.Full_Text)
        return false;
    if (s1.Short_Text!=s2.Short_Text)
        return false;
    if (s1.Station_Type!=s2.Station_Type)
        return false;
    if (s1.Call_Sign!=s2.Call_Sign)
        return false;
    if (s1.PI_String!=s2.PI_String)
        return false;
    if (s1.Group!=s2.Group)
        return false;
    if (s1.TextFlag!=s2.TextFlag)
        return false;
    return true;
};

inline bool operator!= (const RBDS_Output_struct& s1, const RBDS_Output_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<RBDS_Output_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    RBDS_Output_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}


#endif
