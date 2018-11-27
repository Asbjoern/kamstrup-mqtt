#include "mbusparser.h"

std::vector<VectorView> getFrames(const std::vector<uint8_t>& data)
{
  std::vector<VectorView> result;
  if (data.size() < 3) {
    return result;
  }
  size_t offset = 0;
  while (true) {
    //std::cout << "Now searching from offset=" << offset << std::endl;
    size_t start = 0;
    bool startFound = false;
    for (size_t i = offset; i < data.size()-1; ++i) {
      if (data[i] == 0x7E && data[i+1] != 0x7E) {
	startFound = true;
	start = i;
	break;
      }
    }
    if (!startFound) {
      return result;
    }
    size_t end = 0;
    bool endFound = false;
    for (size_t i = start+1; i < data.size(); ++i) {
      if (data[i] == 0x7E) {
	endFound = true;
	end = i+1;
	break;
      }
    }
    if (startFound && endFound && (end-start > 2)) {
      //std::cout << "Start: " << start << " End: " << end << std::endl;
      result.push_back(VectorView(data, start, end-start));
    }
    offset = end+1;
  }
  return result;
}

int find(const VectorView& haystack,
	 const std::vector<uint8_t>& needle)
{
  for (size_t i = 0; i < haystack.size(); ++i) {
    for (size_t j = 0; j < needle.size(); ++j) {
      if (memcmp(&haystack[i], &needle[j], needle.size()-j) == 0) {
	return i;
      } else {
	break;
      }
    }
  }
  return -1;
}

uint32_t getObisValue(const VectorView& frame,
		      uint8_t codeA,
		      uint8_t codeB,
		      uint8_t codeC,
		      uint8_t codeD,
		      uint8_t codeE,
		      uint8_t codeF,
		      uint8_t size,
		      bool & success)
{
  success = false;
  std::vector<uint8_t> theObis = { 0x09, 0x06, codeA, codeB, codeC, codeD, codeE, codeF };
  int indexOfData = find(frame, theObis);
  if (indexOfData >= 0) {
    //std::cout << "IndexOfActivePower: " << indexOfData << std::endl;
    const uint8_t * theBytes = &(frame[0]) + indexOfData + theObis.size() + 1;
    // for (int i = 0; i < 20; ++i) {
    //   std::cout << "Byte " << std::hex << i << ": " << (int)theBytes[i] << std::dec << std::endl;
    // }
    if (size == 2) {
      success = true;
      return
	(uint32_t)theBytes[0] << (8*1) |
	(uint32_t)theBytes[1];
    } else if (size == 3) {
      success = true;
      return
	(uint32_t)theBytes[0] << (8*2) |
	(uint32_t)theBytes[1] << (8*1) |
	(uint32_t)theBytes[2];
    } else if (size == 4) {
      success = true;
      return
	(uint32_t)theBytes[0] << (8*3) |
	(uint32_t)theBytes[1] << (8*2) |
	(uint32_t)theBytes[2] << (8*1) |
	(uint32_t)theBytes[3];
    }
  }
  return 0;
}

enum PowerType {
  ACTIVE_POWER_PLUS = 1,
  ACTIVE_POWER_MINUS = 2,
  REACTIVE_POWER_PLUS = 3,
  REACTIVE_POWER_MINUS = 4,
  CURRENT_L1 = 31, // 0x1F
  CURRENT_L2 = 51, // 0x33
  CURRENT_L3 = 71, // 0x47
  VOLTAGE_L1 = 32, // 0x20
  VOLTAGE_L2 = 52, // 0x34
  VOLTAGE_L3 = 72 // 0x48
};

uint32_t getPower(const VectorView& frame,
		  PowerType type,
		  bool& success)
{
  switch (type) {
  case VOLTAGE_L1:
  case VOLTAGE_L2:
  case VOLTAGE_L3:
    return getObisValue(frame, 1, 1, type, 7, 0, 0xff, 2, success);
  default:
    return getObisValue(frame, 1, 1, type, 7, 0, 0xff, 4, success);
  }
}

MeterData parseMbusFrame(const VectorView& frame)
{
  MeterData result;
  unsigned int frameFormat = frame[1] & 0xF0;
  //std::cout << "Frame format: " << (int)frameFormat << std::endl;
  size_t messageSize = ((frame[1] & 0x0F) << 8) | frame[2];
  //std::cout << "Message size: " << (uint32_t)messageSize << std::endl;
  if (frame.front() == 0x7E && frame.back() == 0x7E) {
    if (frameFormat == 0xA0) {
      // TODO: Parse header
      // TODO: Parse datetime
      // TODO: Parse elements sequentially
      result.activePowerPlus = getPower(frame, ACTIVE_POWER_PLUS, result.activePowerPlusValid);
      result.activePowerMinus = getPower(frame, ACTIVE_POWER_MINUS, result.activePowerMinusValid);
      result.reactivePowerPlus = getPower(frame, REACTIVE_POWER_PLUS, result.reactivePowerPlusValid);
      result.reactivePowerMinus = getPower(frame, REACTIVE_POWER_MINUS, result.reactivePowerMinusValid);
      bool success = false;
      result.currentL1 = getPower(frame, CURRENT_L1, success)/100.0;
      if (!success) {
	result.currentL1 = NAN;
      }
      result.currentL2 = getPower(frame, CURRENT_L2, success)/100.0;
      if (!success) {
	result.currentL2 = NAN;
      }
      result.currentL3 = getPower(frame, CURRENT_L3, success)/100.0;
      if (!success) {
	result.currentL3 = NAN;
      }
      result.voltageL1 = getPower(frame, VOLTAGE_L1, result.voltageL1Valid);
      result.voltageL2 = getPower(frame, VOLTAGE_L2, result.voltageL2Valid);
      result.voltageL3 = getPower(frame, VOLTAGE_L3, result.voltageL3Valid);
    }
  }
  return result;
}