#pragma once
#include <Preferences.h>
#include "../core/IComponent.h"

// Pairing and bearer-token auth for the HTTP API.
//
//  1. startPairing() makes a 6-digit code, valid for 2 minutes, shown to the user.
//  2. confirmPairing(code) checks it and returns a random 256-bit token once.
//     Only the token's SHA-256 is kept in flash, so a flash dump can't reveal it.
//  3. Later requests send "Authorization: Bearer <token>", checked by verifyToken().
//  5 wrong codes lock pairing for 5 minutes.
class DeviceAuth : public IComponent {
public:
  enum class PairResult : uint8_t { Ok, WrongCode, LockedOut };

  bool begin() override;
  const char* name() const override { return "Auth"; }

  bool isPaired() const { return _tokenHash.length() > 0; }

  // Seconds until pairing is allowed again, 0 when not locked out.
  uint32_t retryAfterS() const;
  uint8_t  attemptsLeft() const { return MAX_FAILURES - _failures; }

  bool       startPairing();  // false while locked out
  PairResult confirmPairing(const char* code, String& tokenOut);
  bool       verifyToken(const String& token) const;
  void       unpair();

  // True while a pairing code is waiting to be confirmed (shown on the LCD).
  bool        pairingActive() const;
  const char* pairingCode() const { return _code; }
  uint32_t    codeSecondsLeft() const;  // 0 when no code is active

private:
  static constexpr uint8_t MAX_FAILURES = 5;

  void registerFailure();

  Preferences   _prefs;
  String        _tokenHash;            // hex SHA-256 of the current token
  char          _code[7] = "";         // active pairing code, "" when none
  unsigned long _codeSince   = 0;
  unsigned long _lockedSince = 0;
  bool          _locked      = false;
  uint8_t       _failures    = 0;
};
