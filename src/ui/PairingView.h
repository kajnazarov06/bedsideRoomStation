#pragma once
#include "../core/ITextDisplay.h"
#include "../net/DeviceAuth.h"

// Shows the active pairing code and its countdown on a 2-line display:
//   "Pairing code:"
//   "123456     118s"
class PairingView {
public:
  PairingView(ITextDisplay& display, const DeviceAuth& auth);

  bool isActive() const { return _auth.pairingActive(); }

  // Redraws only when the code or the countdown changed, so it is cheap to
  // call on every loop pass.
  void update();

private:
  ITextDisplay&     _display;
  const DeviceAuth& _auth;
  char              _shownCode[7] = "";
  uint32_t          _shownSecondsLeft = 0;
};
