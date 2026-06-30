<?hh

/**
 * Bitmask flags controlling which warmup phases run during sandbox jumpstart
 * deserialization. Combine with bitwise OR to enable multiple phases.
 *
 * Values match the C++ SBWarmupFlags enum in prof-data-serialize.h.
 */
enum SBWarmupFlag: int {
  PRELOAD = 1;  // Parallel unit preloading
  JIT     = 2;  // Enqueue translations for async JIT compilation
  APC     = 4;  // APC key warmup via ExtensionRegistry data
  ALL     = 7;  // All phases enabled (PRELOAD | JIT | APC)
}

/**
 * Serializes sandbox profile. A profile can be serialized only once for the
 * lifetime of the HHVM process.
 *
 * @param string $sb_root - PHP root directory of the sandbox.
 * @param string $prof_path - Full path of the profile file.
 * @return string - Status of the serialization attempt.
 */
<<__Native>>
function sb_profile_ser(string $sb_root, string $prof_path): ?string;
/**
 * Deserializes sandbox profile. A profile can be deserialized only once for the
 * lifetime of the HHVM process.
 *
 * @param string $sb_root - PHP root directory of the sandbox.
 * @param string $prof_path - Full path of the profile file.
 * @param int $warmup - Bitmask of SBWarmupFlag values controlling which warmup
 *   phases to enable. Defaults to SBWarmupFlag::ALL (all phases).
 * @return string - Status of the deserialization attempt.
 */
<<__Native>>
function sb_profile_deser(
  string $sb_root,
  string $prof_path,
  /* SBWarmupFlag bitmask */ int $warmup = 7 /* SBWarmupFlag::ALL */,
): ?string;
