<?hh

/** Set a callback function to be called whenever a function is entered or
 * exited. Takes 3 args, the function name, the mode (enter or exit), and an
 * array describing the frame.
 * @param mixed $callback - Profiler function to call or null to disable
 * @param int $flags - Controls when it should get called back and with what
 * @param vec<string> $functions - Only receive callbacks on these functions.
 *                                 In effect only when it's not empty.
 */
<<__Native("NoRecording")>>
function fb_setprofile(
  mixed $callback,
  int $flags = SETPROFILE_FLAGS_DEFAULT,
  vec<string> $functions = vec[],
): void;
