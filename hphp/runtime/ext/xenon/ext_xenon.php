<?hh

namespace HH {

type XenonSample = shape(
  'time' => int,
  'stack' => array,
  'phpStack' => array,
  'ioWaitSample' => bool,
);

/**
 * Gather all of the stack traces this request thread has captured by now.
 * Does not clear the stored stacks.
 *
 * @return array - an array of shapes with the following keys:
 *  'time' - unixtime when the snapshot was taken
 *  'stack' - stack trace formatted as debug_backtrace()
 *  'phpStack' - array with the following keys: 'function', 'file', 'line'
 *  'ioWaitSample' - the snapshot occurred while request was in asio scheduler
 *
 *  It is possible for the output of this function to change in the future.
 */
<<__Native>>
function xenon_get_data(): array<XenonSample>;

}
