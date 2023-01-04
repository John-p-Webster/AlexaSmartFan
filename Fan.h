#ifndef _FAN_H_
#define _FAN_H_

#include <SinricProDevice.h>
#include <Capabilities/RangeController.h>
#include <Capabilities/ToggleController.h>

class Fan 
: public SinricProDevice
, public RangeController<Fan>
, public ToggleController<Fan> {
  friend class RangeController<Fan>;
  friend class ToggleController<Fan>;
public:
  Fan(const String &deviceId) : SinricProDevice(deviceId, "Fan") {};
};

#endif
