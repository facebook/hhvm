#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

extern "C" {

/// This function is for use by C/C++ code that wants to test whether the
/// current session is elevated.  The return value is 0 for a non-privileged
/// process and non-zero for a privileged process.
/// If an error occurs while obtaining this information, the program will
/// terminate.
int32_t deelevate_is_privileged_process();

/// This function is for use by C/C++ code that wants to ensure that execution
/// will only continue if the current token has a Normal privilege level.
/// This function will attempt to re-execute the program in the appropriate
/// context.
/// This function will only return if the current context has normal privs.
void deelevate_requires_normal_privileges();

/// This function is for use by C/C++ code that wants to ensure that execution
/// will only continue if the current token has an Elevated privilege level.
/// This function will attempt to re-execute the program in the appropriate
/// context.
/// This function will only return if the current context has Elevated or
/// High Integrity Admin privs.
void deelevate_requires_elevated_privileges();

} // extern "C"
