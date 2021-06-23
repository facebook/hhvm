<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\_Private;

/**
 * Verifies that the `$offset` is within plus/minus `$length`. Returns the
 * offset as a positive integer.
 */
function validate_offset(int $offset, int $length)[]: int {
  $original_offset = $offset;
  if ($offset < 0) {
    $offset += $length;
  }
  invariant(
    $offset >= 0 && $offset <= $length,
    'Offset %d was out-of-bounds for length %d',
    $original_offset,
    $length,
  );
  return $offset;
}

/**
 * Verifies that the `$offset` is not less than minus `$length`. Returns the
 * offset as a positive integer.
 */
function validate_offset_lower_bound(int $offset, int $length)[]: int {
  $original_offset = $offset;
  if ($offset < 0) {
    $offset += $length;
  }
  invariant(
    $offset >= 0,
    'Offset %d was out-of-bounds for length %d',
    $original_offset,
    $length,
  );
  return $offset;
}

function boolval(mixed $val)[]: bool {
  return (bool)$val;
}

const string ALPHABET_ALPHANUMERIC =
  '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';

/**
 * Stop eager execution of an async function.
 *
 * ==== ONLY USE THIS IN HSL IMPLEMENTATION AND TESTS ===
 */
function stop_eager_execution(): RescheduleWaitHandle {
  return RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}
