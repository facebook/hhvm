<?hh

/**
 * Serializes sandbox profile. A profile can be serialized only once for the
 * lifetime of the HHVM process.
 *
 * @param string $sb_root - PHP root directory of the sandbox.
 * @param string $prof_path - Full path of the profile file.
 * @return string - Status of the serialization attempt.
 */
<<__Native>>
function sb_profile_ser(string $sb_root, string $prof_path): string;
/**
 * Deerializes sandbox profile. A profile can be deserialized only once for the
 * lifetime of the HHVM process.
 *
 * @param string $sb_root - PHP root directory of the sandbox.
 * @param string $prof_path - Full path of the profile file.
 * @return string - Status of the deserialization attempt.
 */
<<__Native>>
function sb_profile_deser(string $sb_root, string $prof_path): string;
