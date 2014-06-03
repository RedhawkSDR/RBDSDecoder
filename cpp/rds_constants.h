/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK.
 *
 * REDHAWK is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

/* see page 59, Annex C, table C.1 in the standard
* offset word C' has been put at the end */
static const unsigned int offset_pos[5]={0,1,2,3,2};
static const unsigned int offset_word[5]={252,408,360,436,848};
static const unsigned int syndrome[5]={383,14,303,663,748};
static const char * const offset_name[]={"A","B","C","D","C'"};

/* page 95, Annex F in the standard */
/* UPDATED FOR RBDS */
const std::string pty_table[32]={
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
"Clasical",
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


