#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of RBDSDecoder.
#
# RBDSDecoder is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# RBDSDecoder is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.Boston, MA 02110-1301, USA.
#

import unittest
import ossie.utils.testing
from ossie.utils import sb
import time

# A list to hold the responses the message sink
# receives
responses = []

# The callback function used by the message sink
# to store its responses
def msgCB(msgId, msgStr):
	responses.append(msgStr)

# A convenience method for printing and logging
def display(f, string):
	f.write(string + '\n')
	print string

# Perform modulo-2 polynomial division (Not as
# efficient as the method used by the component,
# but should be more intuitive)
def polyDivide(dividend, divisor):
	binDividend = bin(dividend)[2:]
	binDivisor = bin(divisor)[2:]
	count = 0
	remainder = ''

	while True:
		difference = len(binDivisor) - len(remainder)
		remaining = len(binDividend) - count

		# Enough bits are available to perform
		# a modulo-2 subtraction
		if remaining >= difference:
			remainder += binDividend[count:count+difference]
			count += difference
		# Append the rest of the dividend to the
		# remainder and front pad the result with
		# zeroes so it's of the correct length		
		else:
			remainder += binDividend[count:count+remaining]
			count += remaining
			difference -= remaining

			remainder = '0' * difference + remainder
			count += difference

			break

		# Modulo-2 subtraction = Modulo-2 addition = XOR
		remainder = bin(int(remainder, 2) ^ divisor)[2:]

		if count >= len(binDividend):
			break

	return int(remainder, 2)

# Convert a number to a binary string with
# the desired number of bits in the string
def toBinary(number, count):
	binNumber = bin(number)[2:]

	string = '0' * (count - len(binNumber))

	string += binNumber

	return string

# The calculation used to convert the 16-bit
# data word into the CRC encoded 26-bit block
def calculateBlock(m, x_10, generator, offset_word, length = 26):
	return toBinary((m * x_10) ^ (polyDivide(m * x_10, generator)) ^ offset_word, length)

def main():
	global responses
	f = open('unit_test.log', 'w')

	display(f, "*********************************")
        display(f, "***** RBDSDecoder Unit Test *****")
        display(f, "*********************************")

	# Launch the component and the input sources and message sink
	display(f, "\n****** Creating Component *****")
	test_component = sb.launch('../RBDSDecoder.spd.xml', execparams={'DEBUG_LEVEL':5})
	binarySource = sb.DataSource()
	messageSink = sb.MessageSink(messageCallback=msgCB)

	# Connect the output of the output of the binarySource to the
	# input of the RBDSDecoder.  Connect the output of the 
	# RBDSDecoder to the input of the message sink
	display(f, "\n***** Creating Connections *****")
	binarySource.connect(test_component, providesPortName='dataIn')
	test_component.connect(messageSink, providesPortName='msgIn')
	display(f, "Connections created")

	display(f, "\n******** Generating Data ********")
	# Generate RBDS type 0 and 2 messages
	dummyBlocks = []
	typeABlocks = []
	typeBBlocks = []
	# RBDS Standard, Annex A, Page 70
	offset_words = {'A' : int('0b0011111100', 2), 'B' : int('0b0110011000', 2), 'C' : int('0b0101101000', 2), 'C;' : int('0b1101010000', 2), 'D' : int('0b0110110100', 2)}
	# RBDS Standard, Annex B, Page 71
	generator_poly = int('0b10110111001', 2)
	x_10 = int('0b10000000000', 2)
	# The program identification code for WKLR
	PI_code = 0x703F
	# This indicates the group number being sent (only 0 and 2)
	# supported
	group_type_code = '0000'
	# Type A: B0 = 0, Type B: B0 = 1
	B0 = 0b0
	# These two bits indicate traffic information (ignored by the
	# component)
	TP = 0b0
	TA = 0b0
	# Indication of the program type
	PTY = 0b00110
	# Speech: M_S = 0, Music: M_S = 1
	M_S = 0b0
	# Indication of dynamic PTY and Message (Only message used by
	# component)
	DI_segment = 0b100
	# Alternate frequency information (RBDS Standard, 3.2.1.6.1,
	# Page 42) Unsupported by Component
	AF1 = 0x00
	AF2 = 0x00
	# The Program Service indicated additional information about
	# the station
	PS_Name = "TESTING!"
	# Used to clear the current radio text if a transition occurs
	text_AB = 0b0
	# This indicates where the received characters should be 
	# placed in the radio text sequence
	text_address = 0b0000
	# Type A message
	radio_text_64 = "This is a test!1This is a test!2This is a test!3This is a test!\r"
	# Type B message	
	radio_text_32 = "This is a test!4This is a test!\r"

	# The first blocks will not actually be processed, they're just
	# for syncing (Really only need two of these, but for completeness,
	# send an entire group)
	m = int('0000000000000001', 2)
	c = calculateBlock(m, x_10, generator_poly, offset_words['A'])
	for char in c:
		dummyBlocks.append(int(char))

	c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
	for char in c:
		dummyBlocks.append(int(char))

	c = calculateBlock(m, x_10, generator_poly, offset_words['C'])
	for char in c:
		dummyBlocks.append(int(char))

	c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
	for char in c:
		dummyBlocks.append(int(char))

	# First try group A blocks, alternating between group 0 and 2
	while text_address < 16:
		# The first block in a group is always the same,
		# regardless of the group type
		m = PI_code
		c = calculateBlock(m, x_10, generator_poly, offset_words['A'])
		for char in c:
			typeABlocks.append(int(char))

		if group_type_code == '0000':
			# Encode the group type, the A/B indicator,
			# the TP indicator, the Program Type, the
			# TA indicator, the MS indicator, and the 
			# Dynamic Information Segment
			m = int(group_type_code + bin(B0)[2:] + bin(TP)[2:] + toBinary(PTY, 5) + bin(TA)[2:] + bin(M_S)[2:] + toBinary(DI_segment, 3), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
			for char in c:
				typeABlocks.append(int(char))

			# Encode the Alternate Frequency Information
			m = int(toBinary(AF1, 8) + toBinary(AF2, 8), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['C'])
		        for char in c:
		                typeABlocks.append(int(char))

			# Encode the Program service message
			m = int(toBinary(int(ord(PS_Name[2 * (DI_segment - 8) + 0])), 8) + toBinary(int(ord(PS_Name[2 * (DI_segment - 8) + 1])), 8), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
		        for char in c:
		                typeABlocks.append(int(char))
			
			DI_segment = 4 + (DI_segment + 1) % 4
			group_type_code = '0010'
		elif group_type_code == '0010':
			# Encode the group type, the A/B indicator,
			# the TP indicator, the Program Type, the
			# text clear flag, and the text address
			m = int(group_type_code + bin(B0)[2:] + bin(TP)[2:] + toBinary(PTY, 5) + bin(text_AB)[2:] + toBinary(text_address, 4), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
			for char in c:
				typeABlocks.append(int(char))

			# Encode two characters of the radio
			# text
			m = int(toBinary(int(ord(radio_text_64[text_address * 4 + 0])), 8) + toBinary(int(ord(radio_text_64[text_address * 4 + 1])), 8), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['C'])
        		for char in c:
                		typeABlocks.append(int(char))

			# Encode two more characters of the
			# radio text 
			m = int(toBinary(int(ord(radio_text_64[text_address * 4 + 2])), 8) + toBinary(int(ord(radio_text_64[text_address * 4 + 3])), 8), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
        		for char in c:
                		typeABlocks.append(int(char))
			
			text_address += 1
			group_type_code = '0000'

	# Now try group B blocks, alternating between group 0 and 2
	B0 = 0b1
	# Clear the radio text
	text_AB = 0b1
	# Reset the text address
	text_address = 0b0000

	while text_address < 16:
		# The first block in a group is always the same,
		# regardless of the group type
		m = PI_code
		c = calculateBlock(m, x_10, generator_poly, offset_words['A'])
		for char in c:
			typeBBlocks.append(int(char))

		if group_type_code == '0000':
			# Encode the group type, the A/B indicator,
			# the TP indicator, the Program Type, the
			# TA indicator, the MS indicator, and the 
			# Dynamic Information Segment
			m = int(group_type_code + bin(B0)[2:] + bin(TP)[2:] + toBinary(PTY, 5) + bin(TA)[2:] + bin(M_S)[2:] + toBinary(DI_segment, 3), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
			for char in c:
				typeBBlocks.append(int(char))

			# Type B groups send the PI code again
			m = PI_code
			c = calculateBlock(m, x_10, generator_poly, offset_words['C;'])
			for char in c:
				typeBBlocks.append(int(char))

			# Encode the Program service message
			m = int(toBinary(int(ord(PS_Name[2 * (DI_segment - 8) + 0])), 8) + toBinary(int(ord(PS_Name[2 * (DI_segment - 8) + 1])), 8), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
		        for char in c:
		                typeBBlocks.append(int(char))
			
			DI_segment = 4 + (DI_segment + 1) % 4
			group_type_code = '0010'
		elif group_type_code == '0010':
			# Encode the group type, the A/B indicator,
			# the TP indicator, the Program Type, the
			# text clear flag, and the text address
			m = int(group_type_code + bin(B0)[2:] + bin(TP)[2:] + toBinary(PTY, 5) + bin(text_AB)[2:] + toBinary(text_address, 4), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
			for char in c:
				typeBBlocks.append(int(char))

			# Type B groups send the PI code again
			m = PI_code
			c = calculateBlock(m, x_10, generator_poly, offset_words['C;'])
			for char in c:
				typeBBlocks.append(int(char))

			# Encode two characters of the radio
			# text
			m = int(toBinary(int(ord(radio_text_32[text_address * 2 + 0])), 8) + toBinary(int(ord(radio_text_32[text_address * 2 + 1])), 8), 2)
			c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
        		for char in c:
                		typeBBlocks.append(int(char))
			
			text_address += 1
			group_type_code = '0000'

	display(f, "\n******* Starting Component ******")
        sb.start()
	messageSink.start()
        display(f, "Component started")
	display(f, "**** Sending Sync Blocks ****")
	# The sample rate is set to 1187.5, which is what a
	# real RBDS Decoder would anticipate, but this 
	# component ignores sample rate
	binarySource.push(dummyBlocks, False, 'Test', 1187.5, False, [], None)

	display(f, "** Testing Group 0, Type A **")
	display(f, "** Testing Group 2, Type A **")
	binarySource.push(typeABlocks, False, 'Test', 1187.5, False, [], None)
	time.sleep(0.25)

	type_A_count_test = True
	type_A_full_text_test = True
	type_A_short_text_test = True
	type_A_station_test = True
	type_A_callsign_test = True
	type_A_pi_test = True
	type_A_text_flag_test = True
	type_A_group_num_test = True

	type_A_count_message = ""
	type_A_full_text_message = ""
	type_A_short_text_message = ""
	type_A_station_message = ""
	type_A_callsign_message = ""
	type_A_pi_message = ""
	type_A_text_flag_message = ""
	type_A_group_num_message = ""

	count = 0

	for response in responses:
		for item in response.value.value():
			if item.id == 'Full Text':
				if item.value.value()[count*4:count*4 + 4] != radio_text_64[count*4:count*4 + 4]:
					type_A_full_text_test = False
					type_A_full_text_message = "Expected radio text \'%s\', received \'%s\'" % (radio_text_64[:count*4 + 4], item.value.value()[:count*4 + 4])
			elif item.id == 'Short Text' and count < 4:
				if item.value.value()[count*2:count*2 + 2] != PS_Name[count*2:count*2 + 2]:
					type_A_short_text_test = False
					type_A_short_text_message = "Expected \'%s\', received \'%s\'" % (PS_Name[:count*2 + 2], item.value.value()[:count*2 + 2])
			elif item.id == 'Station Type':
				if item.value.value() != 'Classic Rock':
					type_A_station_test = False
					type_A_station_message = "Expected station type \'%s\', received \'%s\'" % ('Classic Rock', item.value.value())
			elif item.id == 'Call_Sign':
				if item.value.value() != 'WKLR':
					type_A_callsign_test = False
					type_A_callsign_message = "Expected callsign \'%s\', received \'%s\'" % ('WKLR', item.value.value())
			elif item.id == 'PI_String':
				if item.value.value() != '703F':
					type_A_pi_test = False
					type_A_pi_message = "Expected PI code \'%s\', received \'%s\'" % ('703F', item.value.value())
			elif item.id == 'TextFlag' and count > 0:
				if item.value.value() != 'A':
					type_A_text_flag_test = False
					type_A_text_flag_message = "Expected text flag \'%s\', received \'%s\'" % ('A', item.value.value())
			elif item.id == 'Group':
				expected = ''
				if count % 2 == 0:
					expected = '0A'
				else:
					expected = '2A'
				if item.value.value() != expected:
					type_A_group_num_test = False
					type_A_group_num_message = "Expected Group %s on group number %d, received Group %s" % (expected, count, item.value.value())
		count += 1

	if count != 32:
		type_A_count_test = False
		type_A_count_message = "Expected 32 Groups, received %d" % count

	responses = []

	#*************************************************
	display(f, "** Testing Group 0, Type B **")
	display(f, "** Testing Group 2, Type B **")
	binarySource.push(typeBBlocks, False, 'Test', 1187.5, False, [], None)
	time.sleep(0.25)

	type_B_count_test = True
	type_B_full_text_test = True
	type_B_short_text_test = True
	type_B_station_test = True
	type_B_callsign_test = True
	type_B_pi_test = True
	type_B_text_flag_test = True
	type_B_group_num_test = True

	type_B_count_message = ""
	type_B_full_text_message = ""
	type_B_short_text_message = ""
	type_B_station_message = ""
	type_B_callsign_message = ""
	type_B_pi_message = ""
	type_B_text_flag_message = ""
	type_B_group_num_message = ""

	count = 0

	for response in responses:
		for item in response.value.value():
			if item.id == 'Full Text':
				if item.value.value()[count*2:count*2 + 2] != radio_text_32[count*2:count*2 + 2]:
					type_B_full_text_test = False
					type_B_full_text_message = "Expected radio text \'%s\', received \'%s\'" % (radio_text_32[:count*2 + 2], item.value.value()[:count*2 + 2])
			elif item.id == 'Short Text' and count < 4:
				if item.value.value() != PS_Name:
					type_B_short_text_test = False
					type_B_short_text_message = "Expected \'%s\', received \'%s\'" % (PS_Name, item.value.value())
			elif item.id == 'Station Type':
				if item.value.value() != 'Classic Rock':
					type_B_station_test = False
					type_B_station_message = "Expected station type \'%s\', received \'%s\'" % ('Classic Rock', item.value.value())
			elif item.id == 'Call_Sign':
				if item.value.value() != 'WKLR':
					type_B_callsign_test = False
					type_B_callsign_message = "Expected callsign \'%s\', received \'%s\'" % ('WKLR', item.value.value())
			elif item.id == 'PI_String':
				if item.value.value() != '703F':
					type_B_pi_test = False
					type_B_pi_message = "Expected PI code \'%s\', received \'%s\'" % ('703F', item.value.value())
			elif item.id == 'TextFlag' and count > 0:
				if item.value.value() != 'B':
					type_B_text_flag_test = False
					type_B_text_flag_message = "Expected text flag \'%s\', received \'%s\'" % ('B', item.value.value())
			elif item.id == 'Group':
				expected = ''
				if count % 2 == 0:
					expected = '0B'
				else:
					expected = '2B'
				if item.value.value() != expected:
					type_B_group_num_test = False
					type_B_group_num_message = "Expected Group %s on group number %d, received Group %s" % (expected, count, item.value.value())
		count += 1

	if count != 32:
		type_B_count_test = False
		type_B_count_message = "Expected 32 Groups, received %d" % count

	display(f, "\n******* Stopping Component ******")
        sb.stop()
        display(f, "Component stopped")

        # Display the results of the unit test
	if type_A_count_test:
		display(f, "\nType A Count Test ...................." + u'\u2714'.encode('utf8'))
	else:
		display(f, "\nType A Count Test ...................." + u'\u2714'.encode('utf8') + '\t' + type_A_count_message)

	if type_A_full_text_test:
		display(f, "Type A Full Text Test ................" + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type A Full Text Test ................" + u'\u2718'.encode('utf8') + '\t' + type_A_full_text_message)

	if type_A_short_text_test:
		display(f, "Type A Short Text Test ..............." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type A Short Text Test ..............." + u'\u2718'.encode('utf8') + '\t' + type_A_short_text_message)

	if type_A_station_test:
		display(f, "Type A Station Type Test.............." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type A Station Type Test ............." + u'\u2718'.encode('utf8') + '\t' + type_A_station_message)

	if type_A_callsign_test:
		display(f, "Type A Call Sign Test ................" + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type A Call Sign Test ................" + u'\u2718'.encode('utf8') + '\t' + type_A_callsign_message)

	if type_A_pi_test:
		display(f, "Type A PI Code Test .................." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type A PI Code Test .................." + u'\u2718'.encode('utf8') + '\t' + type_A_pi_message)

	if type_A_text_flag_test:
		display(f, "Type A Text Flag Test ................" + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type A Text Flag Test ................" + u'\u2718'.encode('utf8') + '\t' + type_A_text_flag_message)

	if type_A_group_num_test:
		display(f, "Type A Group Number Test ............." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type A Short Number Test ............." + u'\u2718'.encode('utf8') + '\t' + type_A_group_num_message)

	#**************************************************************************************************************************
	if type_B_count_test:
		display(f, "\nType B Count Test ...................." + u'\u2714'.encode('utf8'))
	else:
		display(f, "\nType B Count Test ...................." + u'\u2714'.encode('utf8') + '\t' + type_B_count_message)

	if type_B_full_text_test:
		display(f, "Type B Full Text Test ................" + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type B Full Text Test ................" + u'\u2718'.encode('utf8') + '\t' + type_B_full_text_message)

	if type_B_short_text_test:
		display(f, "Type B Short Text Test ..............." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type B Short Text Test ..............." + u'\u2718'.encode('utf8') + '\t' + type_B_short_text_message)

	if type_B_station_test:
		display(f, "Type B Station Type Test.............." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type B Station Type Test ............." + u'\u2718'.encode('utf8') + '\t' + type_B_station_message)

	if type_B_callsign_test:
		display(f, "Type B Call Sign Test ................" + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type B Call Sign Test ................" + u'\u2718'.encode('utf8') + '\t' + type_B_callsign_message)

	if type_B_pi_test:
		display(f, "Type B PI Code Test .................." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type B PI Code Test .................." + u'\u2718'.encode('utf8') + '\t' + type_B_pi_message)

	if type_B_text_flag_test:
		display(f, "Type B Text Flag Test ................" + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type B Text Flag Test ................" + u'\u2718'.encode('utf8') + '\t' + type_B_text_flag_message)

	if type_B_group_num_test:
		display(f, "Type B Group Number Test ............." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Type B Short Number Test ............." + u'\u2718'.encode('utf8') + '\t' + type_B_group_num_message)

	display(f, '\n')
	display(f, "Unit Test Complete")
	
	f.close()

if __name__ == "__main__":
	main()
