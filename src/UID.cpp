#include "UID.hpp"

using namespace fancon;

/// \breif Check that the UID is valid, and of the DeviceType
bool UID::valid(DeviceType devType) const {
  return valid_ && (type & devType) == type;
}

const string UID::getBasePath() const {
  return string(fancon::Util::hwmon_path) + to_string(hw_id) + '/' + dev_name;
}

bool UID::operator==(const UID &other) const {
  return (this->chipname == other.chipname) &&
         (this->dev_name == other.dev_name) && (this->hw_id == other.hw_id);
}

DeviceType UID::getType() {
  const string sensorDN(Util::temp_sensor_label);
  const bool isSensor =
      search(dev_name.begin(), dev_name.end(), sensorDN.begin(), sensorDN.end())
          != dev_name.end();
  const bool isNVIDIA = chipname == Util::nvidia_label;

  return (isNVIDIA)
         ? ((isSensor) ? DeviceType::sensor_nv : DeviceType::fan_nv)
         : ((isSensor) ? DeviceType::sensor : DeviceType::fan);
}

ostream &fancon::operator<<(ostream &os, const UID &u) {
  using namespace fancon::serialization_constants::uid;
  os << u.chipname << cn_esep << to_string(u.hw_id) << hw_id_esep << u.dev_name;
  return os;
}

istream &fancon::operator>>(istream &is, UID &u) {
  string in;
  is >> in;
  std::remove_if(in.begin(), in.end(), [](auto &c) { return isspace(c); });

  using namespace fancon::serialization_constants::uid;
  auto cnBeginIt = in.begin();
  auto cnEndIt = find(cnBeginIt, in.end(), cn_esep);
  auto hwIdBegIt = cnEndIt + 1;
  auto hwIdEndIt = find(cnEndIt, in.end(), hw_id_esep);
  auto devnBegIt = hwIdEndIt + 1;
  auto devnEndIt = in.end();

  // Fail if all UID is incomplete
  if (Util::equalTo({cnEndIt, hwIdEndIt, devnBegIt}, in.end())) {
    if (!in.empty())
      LOG(llvl::error) << "Invalid UID: " << in;

    u.valid_ = false;
    return is;
  }

  u.chipname = string(cnBeginIt, cnEndIt);
  string hwID = string(hwIdBegIt, hwIdEndIt);
  u.hw_id = std::stoi(hwID);
  u.dev_name = string(devnBegIt, devnEndIt);

  return is;
}
