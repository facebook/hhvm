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

const int OBJPROF_FLAGS_DEFAULT = 1;
const int OBJPROF_FLAGS_USER_TYPES_ONLY = 2;
const int OBJPROF_FLAGS_PER_PROPERTY = 4;

/**
 * Gather all of the stack traces this request thread has captured by now.
 * Does not clear the stored stacks.
 *
 * @return array - an array with the following keys:
 *   'time' - unixtime when the snapshot was taken
 *   'stack' - stack trace formatted as debug_backtrace()
 *   'phpStack' - array with the following keys: 'function', 'file', 'line'
 *   'ioWaitSample' - the snapshot occurred while request was in asio scheduler
 *
 * It is possible for the output of this function to change in the future.
 */
function xenon_get_data(): array<array>; // auto-imported from HH namespace

function thread_memory_stats(): array<string, int>; // auto-imported from HH namespace

function objprof_start(): void; // auto-imported from HH namespace

function objprof_get_data(int $flags = OBJPROF_FLAGS_DEFAULT, array $exclude_list = array()): array<string, array>; // auto-imported from HH namespace

function objprof_get_paths(int $flags = OBJPROF_FLAGS_DEFAULT, array $exclude_list = array()): array<string, array>; // auto-imported from HH namespace

function objprof_get_strings(int $min_dup): array<string, array>; // auto-imported from HH namespace

//////////////////////////////////////////////////////////////////
// Heap graph

function heapgraph_create(): resource; // auto-imported from HH namespace

function heapgraph_stats(resource $heapgraph): array<string, int>; // auto-imported from HH namespace

function heapgraph_foreach_node(resource $heapgraph, mixed $callback): void; // auto-imported from HH namespace
function heapgraph_foreach_edge(resource $heapgraph, mixed $callback): void; // auto-imported from HH namespace
function heapgraph_foreach_root(resource $heapgraph, mixed $callback): void; // auto-imported from HH namespace
function heapgraph_dfs_nodes(resource $heapgraph, array<int> $roots, array<int> $skips, mixed $callback): void; // auto-imported from HH namespace
function heapgraph_dfs_edges(resource $heapgraph, array<int> $roots, array<int> $skips, mixed $callback): void; // auto-imported from HH namespace
function heapgraph_node(resource $heapgraph, int $index): array<string, mixed>; // auto-imported from HH namespace
function heapgraph_edge(resource $heapgraph, int $index): array<string, mixed>; // auto-imported from HH namespace
function heapgraph_node_in_edges(resource $heapgraph, int $index): array<array<string, mixed>>; // auto-imported from HH namespace
function heapgraph_node_out_edges(resource $heapgraph, int $index): array<array<string, mixed>>; // auto-imported from HH namespace
