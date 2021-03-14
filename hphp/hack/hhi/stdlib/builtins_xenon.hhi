<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {

type XenonSample = shape(
  'time' => int,
  'timeNano' => int,
  'lastTriggerTimeNano' => int,
  /* HH_FIXME[2071] */
  'stack' => varray,
  /* HH_FIXME[2071] */
  'phpStack' => varray,
  'ioWaitSample' => bool,
  'sourceType' => string,
);

/**
 * Gather all of the stack traces this request thread has captured by now.
 * Does not clear the stored stacks.
 *
 * @return array - an array of shapes with the following keys:
 *   'time' - unixtime when the snapshot was taken
 *   'stack' - stack trace formatted as debug_backtrace()
 *   'phpStack' - an array of shapes with the following keys:
 *     'function', 'file', 'line'
 *   'ioWaitSample' - the snapshot occurred while request was in asio scheduler
 *
 * It is possible for the output of this function to change in the future.
 */
function xenon_get_data(): varray<XenonSample>; // auto-imported from HH namespace

  /**
   * TODO: this will replace xenon_get_data()
   * this function is same as xenon_get_data() except that it deletes the stack
   * traces that are returned
   */
  function xenon_get_and_clear_samples(): varray<XenonSample>;
  /**
   * Returns the number of xenon samples lost so far.
   */
  function xenon_get_and_clear_missed_sample_count(): int;
  /**
   * Return true whether this request can potentially log Xenon stacks
   */
  function xenon_get_is_profiled_request(): bool;

}
