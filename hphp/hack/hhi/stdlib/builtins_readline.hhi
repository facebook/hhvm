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

// Adds a line to the history
function readline_add_history(string $line): bool;

// Clears the history
function readline_clear_history(): bool;

// Registers a completion function
function readline_completion_function(mixed $function): bool;

// Gets/sets various internal readline variables
function readline_info(string $varname, ?string $newvalue = null): mixed;

// Reads the history
function readline_read_history(?string $filename = null): bool;

// Writes the history
function readline_write_history(?string $filename = null): bool;

// Reads a line
function readline(string $prompt): string;
