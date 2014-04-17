<?hh

namespace HH {

/**
 * Gather all of the stack traces this request thread has captured by now
 *
 * @return Array - an Array of (phpStack, asyncStack) as well as
 *  Array["asyncInvalidCount"] which is how many times Xenon snapped while the
 *  async machine was in an invalid state
 *  It is possible for the output of this function to change in the near
 *  future.  If so, it will be documented.
 */

<<__Native>>
function xenon_get_data(): array;

}
