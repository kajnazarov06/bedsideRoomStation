#include "DeviceAuth.h"
#include <mbedtls/sha256.h>
#include <esp_random.h>

namespace {
  constexpr unsigned long CODE_VALID_MS = 2UL * 60 * 1000;
  constexpr unsigned long LOCKOUT_MS    = 5UL * 60 * 1000;
  constexpr const char*   KEY_TOKEN     = "tokenHash";

  void toHex(const uint8_t* bytes, size_t len, char* out) {
    for (size_t i = 0; i < len; i++) sprintf(out + 2 * i, "%02x", bytes[i]);
  }

  String sha256Hex(const char* text) {
    uint8_t digest[32];
    mbedtls_sha256((const uint8_t*)text, strlen(text), digest, 0);
    char hex[65];
    toHex(digest, sizeof(digest), hex);
    return String(hex);
  }

  // Compare without an early exit so timing doesn't leak the matching prefix.
  bool constantTimeEquals(const String& first, const String& second) {
    if (first.length() != second.length()) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < first.length(); i++) diff |= first[i] ^ second[i];
    return diff == 0;
  }
}

bool DeviceAuth::begin() {
  if (!_prefs.begin("auth", false)) return false;
  _tokenHash = _prefs.getString(KEY_TOKEN, "");
  Serial.printf("[AUTH] %s\n", isPaired() ? "paired" : "not paired");
  return true;
}

uint32_t DeviceAuth::retryAfterS() const {
  if (!_locked) return 0;
  unsigned long elapsed = millis() - _lockedSince;
  return elapsed >= LOCKOUT_MS ? 0 : (LOCKOUT_MS - elapsed + 999) / 1000;
}

bool DeviceAuth::pairingActive() const {
  return _code[0] != '\0' && millis() - _codeSince < CODE_VALID_MS;
}

bool DeviceAuth::startPairing() {
  if (retryAfterS() > 0) return false;
  _locked = false;

  snprintf(_code, sizeof(_code), "%06lu", (unsigned long)(100000 + esp_random() % 900000));
  _codeSince = millis();
  Serial.printf("[AUTH] pairing code: %s (valid %lu s)\n", _code, CODE_VALID_MS / 1000);
  return true;
}

void DeviceAuth::registerFailure() {
  if (++_failures < MAX_FAILURES) return;
  _failures = 0;
  _locked = true;
  _lockedSince = millis();
  _code[0] = '\0';
  Serial.println("[AUTH] too many wrong codes, pairing locked");
}

DeviceAuth::PairResult DeviceAuth::confirmPairing(const char* code, String& tokenOut) {
  if (retryAfterS() > 0) return PairResult::LockedOut;
  _locked = false;

  if (!pairingActive() || strcmp(code, _code) != 0) {
    registerFailure();
    return _locked ? PairResult::LockedOut : PairResult::WrongCode;
  }

  uint8_t raw[32];
  esp_fill_random(raw, sizeof(raw));
  char token[65];
  toHex(raw, sizeof(raw), token);

  _tokenHash = sha256Hex(token);
  _prefs.putString(KEY_TOKEN, _tokenHash);
  _code[0] = '\0';
  _failures = 0;
  Serial.println("[AUTH] paired");

  tokenOut = token;
  return PairResult::Ok;
}

bool DeviceAuth::verifyToken(const String& token) const {
  if (!isPaired() || token.isEmpty()) return false;
  return constantTimeEquals(sha256Hex(token.c_str()), _tokenHash);
}

void DeviceAuth::unpair() {
  _tokenHash = "";
  _prefs.remove(KEY_TOKEN);
  Serial.println("[AUTH] unpaired");
}
