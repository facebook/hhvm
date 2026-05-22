<?hh

enum SBWarmupFlag: int {
  PRELOAD = 1;  // Parallel unit preloading
  JIT     = 2;  // Enqueue translations for async JIT compilation
  APC     = 4;  // APC key warmup via ExtensionRegistry data
  ALL     = 7;  // All phases enabled (PRELOAD | JIT | APC)
}

<<__PHPStdLib>>
function sb_profile_ser(string $sb_root, string $prof_path): string;
<<__PHPStdLib>>
function sb_profile_deser(string $sb_root, string $prof_path, int $warmup): string;
