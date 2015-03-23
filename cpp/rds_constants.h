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


#ifndef RDSCONSTANTS_H
#define RDSCONSTANTS_H

#include <map>

/* see page 59, Annex C, table C.1 in the standard
* offset word C' has been put at the end */
static const unsigned int offset_pos[5]={0,1,2,3,2};
static const unsigned int offset_word[5]={252,408,360,436,848};
static const unsigned int syndrome[5]={383,14,303,663,748};
static const char * const offset_name[]={"A","B","C","D","C'"};

/* page 95, Annex F in the standard */
/* UPDATED FOR RBDS */
const std::string rbds_pty_table[32]={
"None",
"News",
"Information",
"Sports",
"Talk",
"Rock",
"Classic_Rock",
"Adult_Hits",
"Soft_Rock",
"Top_40",
"Country",
"Oldies",
"Soft",
"Nostalgia",
"Jazz",
"Classical",
"Rhythm_and_Blues",
"Soft_R_&_B",
"Foreign_Language",
"Religious_Music",
"Religius_Talk",
"Personality",
"Public",
"College",
"Unassigned",
"Unassigned",
"Unassigned",
"Unassigned",
"Unassigned",
"Weather",
"Emergency_Test",
"ALERT!_ALERT!"};

/* page 71, Annex D, table D.1 in the standard */
const std::string pi_country_codes[15][5]={
{"DE","GR","MA","__","MD"},
{"DZ","CY","CZ","IE","EE"},
{"AD","SM","PL","TR","__"},
{"IL","CH","VA","MK","__"},
{"IT","JO","SK","__","__"},
{"BE","FI","SY","__","UA"},
{"RU","LU","TN","__","__"},
{"PS","BG","__","NL","PT"},
{"AL","DK","LI","LV","SI"},
{"AT","GI","IS","LB","__"},
{"HU","IQ","MC","__","__"},
{"MT","GB","LT","HR","__"},
{"DE","LY","YU","__","__"},
{"__","RO","ES","SE","__"},
{"EG","FR","NO","BY","BA"}};

/* page 72, Annex D, table D.2 in the standard */
const std::string coverage_area_codes[16]={
"Local",
"International",
"National",
"Supra-regional",
"Regional 1",
"Regional 2",
"Regional 3",
"Regional 4",
"Regional 5",
"Regional 6",
"Regional 7",
"Regional 8",
"Regional 9",
"Regional 10",
"Regional 11",
"Regional 12"};

const std::string rds_group_acronyms[16]={
"BASIC",
"PIN/SL",
"RT",
"AID",
"CT",
"TDC",
"IH",
"RP",
"TMC",
"EWS",
"___",
"___",
"___",
"___",
"EON",
"___"};

/* page 74, Annex E, table E.1 in the standard: that's the ASCII table!!! */

/* see page 84, Annex J in the standard */
const std::string language_codes[44]={
"Unkown/not applicable",
"Albanian",
"Breton",
"Catalan",
"Croatian",
"Welsh",
"Czech",
"Danish",
"German",
"English",
"Spanish",
"Esperanto",
"Estonian",
"Basque",
"Faroese",
"French",
"Frisian",
"Irish",
"Gaelic",
"Galician",
"Icelandic",
"Italian",
"Lappish",
"Latin",
"Latvian",
"Luxembourgian",
"Lithuanian",
"Hungarian",
"Maltese",
"Dutch",
"Norwegian",
"Occitan",
"Polish",
"Portuguese",
"Romanian",
"Romansh",
"Serbian",
"Slovak",
"Slovene",
"Finnish",
"Swedish",
"Turkish",
"Flemish",
"Walloon"};

/* see page 12 in ISO 14819-1 */
const std::string tmc_duration[8][2]={
{"no duration given", "no duration given"},
{"15 minutes", "next few hours"},
{"30 minutes", "rest of the day"},
{"1 hour", "until tomorrow evening"},
{"2 hours", "rest of the week"},
{"3 hours", "end of next week"},
{"4 hours", "end of the month"},
{"rest of the day", "long period"}};

/* optional message content, data field lengths and labels
* see page 15 in ISO 14819-1 */
const int optional_content_lengths[16]={3,3,5,5,5,8,8,8,8,11,16,16,16,16,0,0};

const std::string label_descriptions[16]={
"Duration",
"Control code",
"Length of route affected",
"Speed limit advice",
"Quantifier",
"Quantifier",
"Supplementary information code",
"Explicit start time",
"Explicit stop time",
"Additional event",
"Detailed diversion instructions",
"Destination",
"RFU (Reserved for future use)",
"Cross linkage to source of problem, or another route",
"Separator",
"RFU (Reserved for future use)"};

const char call_letters[26]={'A','B','C','D','E','F','G','H','I','J','K','L',
		'M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};


struct CALL_MAP{

    static std::map<unsigned int, std::string> create_map()
        {
    	std::map<unsigned int, std::string> callMap;
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
          return callMap;
        }
    static const std::map<unsigned int, std::string> myMap;

};

// Method for taking the int based PI code and converting it to the 4 character Station call sign.
// Taken from: www.w9wi.com/articles/rds.htm
static std::string decode_callsign(unsigned int picode) {

	char call2, call3, call4;
	std::string csign;

	unsigned int code;
	csign="";

	if (picode > 4095) {
		if (picode < 39247) {
			if (picode > 21671) {
				csign+="W";
				code=picode-21672;
			} else {
				csign+="K";
				code=picode-4096;
			}
			call2=code/676;
			code=code-(676*call2);
			call3=code/26;
			call4=code-(26*call3);
			csign += (char)(((int)'A')+call2);
			csign += (char)(((int)'A')+call3);
			csign += (char)(((int)'A')+call4);
		}
	}
			switch(picode) {
				case 49829:
					csign="CIMF";
					break;
				case 17185:
					csign="CJPT";
					break;
				case 39248:
					csign="KEX";
					break;
				case 39249:
					csign="KFH";
					break;
				case 39250:
					csign="KFI";
					break;
				case 39251:
					csign="KGA";
					break;
				case 39252:
					csign="KGO";
					break;
				case 39253:
					csign="KGU";
					break;
				case 39254:
					csign="KGW";
					break;
				case 39255:
					csign="KGY";
					break;
				case 39256:
					csign="KID";
					break;
				case 39257:
					csign="KIT";
					break;
				case 39258:
					csign="KJR";
					break;
				case 39259:
					csign="KLO";
					break;
				case 39260:
					csign="KLZ";
					break;
				case 39261:
					csign="KMA";
					break;
				case 39262:
					csign="KMJ";
					break;
				case 39263:
					csign="KNX";
					break;
				case 39264:
					csign="KOA";
					break;
				case 39268:
					csign="KQV";
					break;
				case 39269:
					csign="KSL";
					break;
				case 39270:
					csign="KUJ";
					break;
				case 39271:
					csign="KVI";
					break;
				case 39272:
					csign="KWG";
					break;
				case 39275:
					csign="KYW";
					break;
				case 39277:
					csign="WBZ";
					break;
				case 39278:
					csign="WDZ";
					break;
				case 39279:
					csign="WEW";
					break;
				case 39281:
					csign="WGL";
					break;
				case 39282:
					csign="WGN";
					break;
				case 39283:
					csign="WGR";
					break;
				case 39285:
					csign="WHA";
					break;
				case 39286:
					csign="WHB";
					break;
				case 39287:
					csign="WHK";
					break;
				case 39288:
					csign="WHO";
					break;
				case 39290:
					csign="WIP";
					break;
				case 39291:
					csign="WJR";
					break;
				case 39292:
					csign="WKY";
					break;
				case 39293:
					csign="WLS";
					break;
				case 39294:
					csign="WLW";
					break;
				case 39297:
					csign="WOC";
					break;
				case 39299:
					csign="WOL";
					break;
				case 39300:
					csign="WOR";
					break;
				case 39304:
					csign="WWJ";
					break;
				case 39305:
					csign="WWL";
					break;
				case 39312:
					csign="KDB";
					break;
				case 39313:
					csign="KGB";
					break;
				case 39314:
					csign="KOY";
					break;
				case 39315:
					csign="KPQ";
					break;
				case 39316:
					csign="KSD";
					break;
				case 39317:
					csign="KUT";
					break;
				case 39318:
					csign="KXL";
					break;
				case 39319:
					csign="KXO";
					break;
				case 39321:
					csign="WBT";
					break;
				case 39322:
					csign="WGH";
					break;
				case 39323:
					csign="WGY";
					break;
				case 39324:
					csign="WHP";
					break;
				case 39325:
					csign="WIL";
					break;
				case 39326:
					csign="WMC";
					break;
				case 39327:
					csign="WMT";
					break;
				case 39328:
					csign="WOI";
					break;
				case 39329:
					csign="WOW";
					break;
				case 39330:
					csign="WRR";
					break;
				case 39331:
					csign="WSB";
					break;
				case 39332:
					csign="WSM";
					break;
				case 39333:
					csign="KBW";
					break;
				case 39334:
					csign="KCY";
					break;
				case 39335:
					csign="KDF";
					break;
				case 39338:
					csign="KHQ";
					break;
				case 39339:
					csign="KOB";
					break;
				case 39347:
					csign="WIS";
					break;
				case 39348:
					csign="WJW";
					break;
				case 39349:
					csign="WJZ";
					break;
				case 39353:
					csign="WRC";
					break;
				case 26542:
					csign="WHFI/CHFI";
					break;
				case 49160:
					csign="CJBC-1";
					break;
				case 49158:
					csign="CBCK";
					break;
				case 52010:
					csign="CBLG";
					break;
				case 52007:
					csign="CBLJ";
					break;
				case 52012:
					csign="CBQT";
					break;
				case 52009:
					csign="CBEB";
					break;
				case 28378:
					csign="WJXY/CJXY";
					break;
				case 941:
					csign="CKGE";
					break;
				case 16416:
					csign="KSFW/CBLA";
					break;
				case 25414:
					csign="WFNY/CFNY";
					break;
				case 27382:
					csign="WILQ/CILQ";
					break;
				case 27424:
					csign="WING/CING";
					break;
				case 26428:
					csign="WHAY/CHAY";
					break;
				case 52033:
					csign="CBA-FM";
					break;
				case 52034:
					csign="CBCT";
					break;
				case 52045:
					csign="CBHM";
					break;
				case 45084:
					csign="CIQM";
					break;
				case 51806:
					csign="CHNI, CJNI, or CKNI";
					break;
				case 12289:
					csign="KLAS (Jamaica)";
					break;
				case 7877:
					csign="CFPL";
					break;
				case 7760:
					csign="ZFKY (Cayman Is.)";
					break;
				case 8151:
					csign="ZFCC (Cayman Is.)";
					break;
				case 12656:
					csign="WAVW";
					break;
				case 7908:
					csign="KTCZ";
					break;
				case 42149:
					csign="KSKZ or KWKR";
					break;
				case 45313:
					csign="XHCTO";
					break;
				case 34784:
					csign="XHTRR";
					break;
			}

			return csign;
}

#endif
