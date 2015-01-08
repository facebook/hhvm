<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * Gather all of the stack traces this request thread has captured by now.
 * Does not clear the stored stacks.
 *
 * @return array - an array with the following keys:
 *   'time' - unixtime when the snapshot was taken
 *   'phpStack' - array with the following keys: 'function', 'file', 'line'
 *   'ioWaitSample' - the snapshot occurred while request was in asio scheduler
 *
 * It is possible for the output of this function to change in the future.
 */
function xenon_get_data(): array<array>; // auto-imported from HH namespace

function objprof_start(): void; // auto-imported from HH namespace

function objprof_get_data(): array<string, array>; // auto-imported from HH namespace

function objprof_get_strings(int $min_dup): array<string, array>; // auto-imported from HH namespace
