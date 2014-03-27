<?hh

namespace __SystemLib {

/**
 * Determine if a debugger is attached to the current thread, and
 * return information about where it is connected from. The client IP
 * and port will be null if the connection is local.
 */
<<__Native>>
function debugger_get_info() : array;

}
