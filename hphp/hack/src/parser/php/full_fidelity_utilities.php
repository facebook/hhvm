<?hh //strict

/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 */

// A wrapper around the built-in exec with a nicer signature.
// * returns a result rather than filling an out-parameter
// * throws on error
function execute(string $command) : array<string> {
  $results = array();
  exec($command, $results, $errorcode);
  if ($errorcode != 0)
    throw new Exception("execute of '$command' failed with code '$errorcode'.");
  return $results;
}

function fold_map<TInput, TOutput, TAccumulation>(
    Traversable<TInput> $items,
    (function (TInput, TAccumulation): TOutput) $mapper,
    (function (TInput, TAccumulation): TAccumulation) $accumulator,
    TAccumulation $initial,
  ): array<TOutput> {
  $acc = $initial;
  $result = [];
  foreach($items as $item) {
    array_push($result, $mapper($item, $acc));
    $acc = $accumulator($item, $acc);
  }
  return $result;
}
