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

namespace {

  /* Request authentication token from the client. The token is empty in case of
   * error.
   */
  <<__HipHopSpecific, __Native("NoFCallBuiltin")>>
  function hphpd_auth_token(): string;

  /* Sets a hard breakpoint. When a debugger is running, this line of code will
   * break into debugger, if condition is met. If there is no debugger that's
   * attached, it will not do anything.
   * @param bool $condition - If true, break, otherwise, continue.
   */
  <<__HipHopSpecific, __Native("NoFCallBuiltin")>>
  function hphpd_break(bool $condition = true): void;


  /* Quickly determine if a debugger is attached to this process and configured
   * to debug this thread.
   * @return bool - TRUE if a debugger is attached, FALSE if not.
   */
  <<__HipHopSpecific, __Native("NoFCallBuiltin")>>
  function hphp_debugger_attached(): bool;

  /* Invokes a hard breakpoint. This routine will break into the debugger if
   * and only if the debugger is enabled and a debugger client is currently
   * attached.
   * @param bool $condition - Optional condition. If specified, the debugger
   *  will only break if the condition evaluates to true.
   * @return bool - TRUE if the program successfully broke in (and has since
   *   resumed), FALSE if no debugger was attached.
   */
  <<__HipHopSpecific, __Native("NoFCallBuiltin")>>
  function hphp_debug_break(bool $condition = true): bool;
}
