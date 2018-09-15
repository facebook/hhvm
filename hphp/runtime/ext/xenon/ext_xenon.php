<?hh

namespace HH {

type XenonSample = shape(
  'time' => int,
  'stack' => varray,
  'phpStack' => varray,
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
function xenon_get_data(): varray<XenonSample>;
/**
 * TODO: this will replace xenon_get_data()
 * this function is same as xenon_get_data() except that it deletes the stack
 * traces that are returned
 */
<<__Native>>
function xenon_get_and_clear_samples(): varray<XenonSample>;

/**
 * Returns the number of xenon samples lost so far.
 */
<<__Native>>
function xenon_get_and_clear_missed_sample_count(): int;

/**
 * Return true whether this request can potentially log Xenon stacks
 */
<<__Native>>
function xenon_get_is_profiled_request(): bool;
}
