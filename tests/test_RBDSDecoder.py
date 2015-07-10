#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK RBDSDecoder.
#
# REDHAWK RBDSDecoder is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK RBDSDecoder is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest
import ossie.utils.testing
from ossie.utils import sb
import time
import inspect


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

def whoami():
    return inspect.stack()[1][3]

class ComponentTests(ossie.utils.testing.ScaComponentTestCase):

    # Type A message
    radio_text_64 = "This_is_a_test!1This_is_a_test!2This_is_a_test!3This_is_a_test!_"
    # Type B message    
    radio_text_32 = "This_is_a_test!4This_is_a_test!_"
    # The program identification code for WKLR
    PI_code = 0x703F
    
    # Station Type
    station_type = 'Classic_Rock'
    # The Program Service indicated additional information about
    # the station
    PS_Name = "TESTING!"
    
    call_sign = "WKLR"
    
    # The amount of time needed for all the data to go through the system in seconds
    flushTime = 0.25

    # The callback function used by the message sink
    # to store its responses
    def msgCB(self, msgId, msgStr):
        self.responses.append(msgStr)

    def setUp(self):
        print "\nSetting up a Test\n"
        # Generate RBDS type 0 and 2 messages
        self.dummyBlocks = []
        self.typeABlocks = []
        self.typeBBlocks = []
        # A list to hold the responses the message sink
        # receives
        self.responses = []

        # Launch the component and the input sources and message sink
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        self.launch()
        self.binarySource = sb.DataSource()
        self.messageSink = sb.MessageSink(messageCallback=self.msgCB)
        
        # Connect the output of the output of the binarySource to the
        # input of the RBDSDecoder.  Connect the output of the 
        # RBDSDecoder to the input of the message sink
        self.binarySource.connect(self.comp, providesPortName='dataShort_in')
        self.comp.connect(self.messageSink, providesPortName='msgIn')

        self.generateData()
        
        # Now that we have all of our data put together push it into the component
        self.comp.start()
        self.binarySource.start()
        self.messageSink.start()
        
        # The sample rate is set to 1187.5, which is what a
        # real RBDS Decoder would anticipate, but this 
        # component ignores sample rate
        self.binarySource.push(self.dummyBlocks, False, 'Test', 1187.5, False, [], None)


    def tearDown(self):
        print "\nTearing down a Test\n"
        self.responses = []
        self.messageSink.stop()
        self.binarySource.stop()
        self.comp.releaseObject()
        ossie.utils.testing.ScaComponentTestCase.tearDown(self)

    def testATypeCount(self):
        self.binarySource.push(self.typeABlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.typeCountTest()

    def testBTypeCount(self):
        self.binarySource.push(self.typeBBlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.typeCountTest()
        
        
    def typeCountTest(self):
        count = 0
        for response in self.responses:
            count += 1
        self.assertTrue(count == 112, "Did not receive expected number of messages, expected 112, received %s" % count)

    def testATypeFullText(self):
        self.binarySource.push(self.typeABlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.typeFullTextTest(self.radio_text_64)

    def testBTypeFullText(self):
        self.binarySource.push(self.typeBBlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.typeFullTextTest(self.radio_text_32)
        
    def typeFullTextTest(self, matchText):
        type = "RBDS_Output::Full_Text"
        print "Running %s" % (whoami())
        msgReceived = False
        receivedCorrectText = True
        
        for response in self.responses:
            for item in response.value.value():
                if item.id == type:
                    msgReceived = True
                    for a, b in zip(item.value.value(), matchText):
                        if a == " ":
                            break
                        if a != b:
                            receivedCorrectText = False
                        
        self.assertTrue(msgReceived, "Did not receive any %s message types" % type)
        self.assertTrue(receivedCorrectText, "One or more %s messages were not correct" % type)

    def testAShortText(self):
        self.binarySource.push(self.typeABlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.shortTextTest()

    def testBShortText(self):
        self.binarySource.push(self.typeBBlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.shortTextTest()

    def shortTextTest(self):
        type = "RBDS_Output::Short_Text"
        print "Running %s" % (whoami())
        msgReceived = False
        receivedCorrectText = True
        
        for response in self.responses:
            for item in response.value.value():
                if item.id == type:
                    msgReceived = True
                    for a, b in zip(item.value.value(), self.PS_Name):
                        if a == ".":
                            break
                        if a != b:
                            receivedCorrectText = False
                        
        self.assertTrue(msgReceived, "Did not receive any %s message types" % type)
        self.assertTrue(receivedCorrectText, "One or more %s messages were not correct" % type)

    def testAStationType(self):
        self.binarySource.push(self.typeABlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.stationTypeTest()
    
    def testBStationType(self):
        self.binarySource.push(self.typeBBlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.stationTypeTest()

    def stationTypeTest(self):
        type = "RBDS_Output::Station_Type"
        print "Running %s" % (whoami())
        msgReceived = False
        receivedCorrectText = True

        
        for response in self.responses:
            for item in response.value.value():
                if item.id == type and item.value.value() != "None":
                    msgReceived = True
                    for a, b in zip(item.value.value(), self.station_type):
                        if a == ".":
                            break
                        if a != b:
                            receivedCorrectText = False
                            
                        
        self.assertTrue(msgReceived, "Did not receive any %s message types" % type)
        self.assertTrue(receivedCorrectText, "One or more %s messages were not correct" % type)

    def testACallSign(self):
        self.binarySource.push(self.typeABlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.callSignTest()

    def testBCallSign(self):
        self.binarySource.push(self.typeBBlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.callSignTest()
        
    def callSignTest(self):
        type = "RBDS_Output::Call_Sign"
        print "Running %s" % (whoami())
        msgReceived = False
        receivedCorrectText = True
        
        for response in self.responses:
            for item in response.value.value():
                if item.id == type:
                    msgReceived = True
                    for a, b in zip(item.value.value(), self.call_sign):
                        if a != b:
                            receivedCorrectText = False
                            
                        
        self.assertTrue(msgReceived, "Did not receive any %s message types" % type)
        self.assertTrue(receivedCorrectText, "One or more %s messages were not correct" % type)

    def testAPIString(self):
        self.binarySource.push(self.typeABlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.piStringTest()

    def testBPIString(self):
        self.binarySource.push(self.typeBBlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.piStringTest()
        
    def piStringTest(self):
        type = "RBDS_Output::PI_String"
        print "Running %s" % (whoami())
        msgReceived = False
        receivedCorrectText = True
        
        for response in self.responses:
            for item in response.value.value():
                if item.id == type:
                    msgReceived = True
                    for a, b in zip(item.value.value(), str(self.PI_code)):
                        if a != b:
                            receivedCorrectText = False

        self.assertTrue(msgReceived, "Did not receive any %s message types" % type)
        self.assertTrue(receivedCorrectText, "One or more %s messages were not correct" % type)

    def testAGroupFlag(self):
        self.binarySource.push(self.typeABlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.groupFlagTest('A')

    def testBGroupFlag(self):
        self.binarySource.push(self.typeBBlocks, False, 'Test', 1187.5, False, [], None)
        time.sleep(self.flushTime) # Wait a second for the data to process
        self.groupFlagTest('B')

    def groupFlagTest(self, group):
        type = "RBDS_Output::Group"
        print "Running %s" % (whoami())
        msgReceived = False
        receivedCorrectText = True
        
        for response in self.responses:
            for item in response.value.value():
                if item.id == type:
                    msgReceived = True
                    if (len(item.value.value()) < 3 or item.value.value()[2] != group):
                            receivedCorrectText = False

        self.assertTrue(msgReceived, "Did not receive any %s message types" % type)
        self.assertTrue(receivedCorrectText, "One or more %s messages were not correct" % type)

    def generateData(self):
        # RBDS Standard, Annex A, Page 70
        offset_words = {'A' : int('0b0011111100', 2), 'B' : int('0b0110011000', 2), 'C' : int('0b0101101000', 2), 'C;' : int('0b1101010000', 2), 'D' : int('0b0110110100', 2)}
        # RBDS Standard, Annex B, Page 71
        generator_poly = int('0b10110111001', 2)
        x_10 = int('0b10000000000', 2)

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
        # Used to clear the current radio text if a transition occurs
        text_AB = 0b0
        # This indicates where the received characters should be 
        # placed in the radio text sequence
        text_address = 0b0000
        
        # The first blocks will not actually be processed, they're just
        # for syncing (Really only need two of these, but for completeness,
        # send an entire group)
        m = int('0000000000000001', 2)
        c = calculateBlock(m, x_10, generator_poly, offset_words['A'])
        for char in c:
            self.dummyBlocks.append(int(char))
        
        c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
        for char in c:
            self.dummyBlocks.append(int(char))
        
        c = calculateBlock(m, x_10, generator_poly, offset_words['C'])
        for char in c:
            self.dummyBlocks.append(int(char))
        
        c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
        for char in c:
            self.dummyBlocks.append(int(char))
           
        # First try group A blocks, alternating between group 0 and 2
        while text_address < 16:
            # The first block in a group is always the same,
            # regardless of the group type
            m = self.PI_code
            c = calculateBlock(m, x_10, generator_poly, offset_words['A'])
            for char in c:
                self.typeABlocks.append(int(char))

            if group_type_code == '0000':
                # Encode the group type, the A/B indicator,
                # the TP indicator, the Program Type, the
                # TA indicator, the MS indicator, and the 
                # Dynamic Information Segment
                m = int(group_type_code + bin(B0)[2:] + bin(TP)[2:] + toBinary(PTY, 5) + bin(TA)[2:] + bin(M_S)[2:] + toBinary(DI_segment, 3), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
                for char in c:
                    self.typeABlocks.append(int(char))

                # Encode the Alternate Frequency Information
                m = int(toBinary(AF1, 8) + toBinary(AF2, 8), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['C'])
                for char in c:
                    self.typeABlocks.append(int(char))

                # Encode the Program service message
                m = int(toBinary(int(ord(self.PS_Name[2 * (DI_segment - 8) + 0])), 8) + toBinary(int(ord(self.PS_Name[2 * (DI_segment - 8) + 1])), 8), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
                for char in c:
                    self.typeABlocks.append(int(char))

                DI_segment = 4 + (DI_segment + 1) % 4
                group_type_code = '0010'
            elif group_type_code == '0010':
                # Encode the group type, the A/B indicator,
                # the TP indicator, the Program Type, the
                # text clear flag, and the text address
                m = int(group_type_code + bin(B0)[2:] + bin(TP)[2:] + toBinary(PTY, 5) + bin(text_AB)[2:] + toBinary(text_address, 4), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
                for char in c:
                    self.typeABlocks.append(int(char))

                # Encode two characters of the radio
                # text
                m = int(toBinary(int(ord(self.radio_text_64[text_address * 4 + 0])), 8) + toBinary(int(ord(self.radio_text_64[text_address * 4 + 1])), 8), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['C'])
                for char in c:
                    self.typeABlocks.append(int(char))

                # Encode two more characters of the
                # radio text 
                m = int(toBinary(int(ord(self.radio_text_64[text_address * 4 + 2])), 8) + toBinary(int(ord(self.radio_text_64[text_address * 4 + 3])), 8), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
                for char in c:
                    self.typeABlocks.append(int(char))

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
            m = self.PI_code
            c = calculateBlock(m, x_10, generator_poly, offset_words['A'])
            for char in c:
                self.typeBBlocks.append(int(char))

            if group_type_code == '0000':
                # Encode the group type, the A/B indicator,
                # the TP indicator, the Program Type, the
                # TA indicator, the MS indicator, and the 
                # Dynamic Information Segment
                m = int(group_type_code + bin(B0)[2:] + bin(TP)[2:] + toBinary(PTY, 5) + bin(TA)[2:] + bin(M_S)[2:] + toBinary(DI_segment, 3), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
                for char in c:
                    self.typeBBlocks.append(int(char))

                # Type B groups send the PI code again
                m = self.PI_code
                c = calculateBlock(m, x_10, generator_poly, offset_words['C;'])
                for char in c:
                    self.typeBBlocks.append(int(char))

                # Encode the Program service message
                m = int(toBinary(int(ord(self.PS_Name[2 * (DI_segment - 8) + 0])), 8) + toBinary(int(ord(self.PS_Name[2 * (DI_segment - 8) + 1])), 8), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
                for char in c:
                    self.typeBBlocks.append(int(char))

                DI_segment = 4 + (DI_segment + 1) % 4
                group_type_code = '0010'
            elif group_type_code == '0010':
                # Encode the group type, the A/B indicator,
                # the TP indicator, the Program Type, the
                # text clear flag, and the text address
                m = int(group_type_code + bin(B0)[2:] + bin(TP)[2:] + toBinary(PTY, 5) + bin(text_AB)[2:] + toBinary(text_address, 4), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['B'])
                for char in c:
                    self.typeBBlocks.append(int(char))

                # Type B groups send the PI code again
                m = self.PI_code
                c = calculateBlock(m, x_10, generator_poly, offset_words['C;'])
                for char in c:
                    self.typeBBlocks.append(int(char))

                # Encode two characters of the radio
                # text
                m = int(toBinary(int(ord(self.radio_text_32[text_address * 2 + 0])), 8) + toBinary(int(ord(self.radio_text_32[text_address * 2 + 1])), 8), 2)
                c = calculateBlock(m, x_10, generator_poly, offset_words['D'])
                for char in c:
                    self.typeBBlocks.append(int(char))

                text_address += 1
                group_type_code = '0000'

if __name__ == "__main__":
    ossie.utils.testing.main("../RBDSDecoder.spd.xml", impl="cpp") # By default only test the x86_64 implementation
