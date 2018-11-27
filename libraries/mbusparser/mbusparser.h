#ifndef MBUSPARSER_H
#define MBUSPARSER_H

#include <vector>
#include <cstring>
#include <cmath>
#include <cstdint>

struct MeterData {
  // Active Power +/-
  uint32_t activePowerPlus = 0;
  bool activePowerPlusValid = false;
  uint32_t activePowerMinus = 0;
  bool activePowerMinusValid = false;

  // Reactive Power +/-
  uint32_t reactivePowerPlus = 0;
  bool reactivePowerPlusValid = false;
  uint32_t reactivePowerMinus = 0;
  bool reactivePowerMinusValid = false;

  // Voltage L1/L2/L3
  uint32_t voltageL1 = 0;
  bool voltageL1Valid = false;
  uint32_t voltageL2 = 0;
  bool voltageL2Valid = false;
  uint32_t voltageL3 = 0;
  bool voltageL3Valid = false;

  // Current L1/L2/L3
  float currentL1 = 0;
  float currentL2 = 0;
  float currentL3 = 0;
};

struct VectorView {
  VectorView(const std::vector<uint8_t>& data, size_t position, size_t size)
    : m_start(&data[position])
    , m_size(size)
  {
  }
  const uint8_t& operator[](size_t pos) const { return m_start[pos]; }
  const uint8_t& front() const { return m_start[0]; }
  const uint8_t& back() const { return m_start[m_size-1]; }
  size_t size() const noexcept { return m_size; }
private:
  const uint8_t* m_start;
  const size_t m_size;
};

std::vector<VectorView> getFrames(const std::vector<uint8_t>& data);
MeterData parseMbusFrame(const VectorView& frame);

#endif
