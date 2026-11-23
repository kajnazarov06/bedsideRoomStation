#include "PairingView.h"

PairingView::PairingView(ITextDisplay& display, const DeviceAuth& auth)
  : _display(display), _auth(auth) {}

void PairingView::update() {
  const char* code = _auth.pairingCode();
  uint32_t secondsLeft = _auth.codeSecondsLeft();
  if (secondsLeft == _shownSecondsLeft && strcmp(code, _shownCode) == 0) return;

  strlcpy(_shownCode, code, sizeof(_shownCode));
  _shownSecondsLeft = secondsLeft;

  char line[17];
  snprintf(line, sizeof(line), "%-6s %8lus", code, (unsigned long)secondsLeft);
  _display.printLine(0, "Pairing code:");
  _display.printLine(1, line);
}
