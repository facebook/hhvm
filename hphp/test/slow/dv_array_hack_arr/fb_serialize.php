<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function round_trip(mixed $val, int $mode, bool $mark_legacy): mixed {
  $err = false;
  if ($mark_legacy) {
    $val = HH\array_mark_legacy($val);
  }
  $result = fb_unserialize(fb_serialize($val, $mode), inout $err, $mode);
  return $result;
}

function intish_cast(dict<arraykey, mixed> $val): dict<arraykey, mixed> {
  $res = dict[];
  foreach ($val as $k => $v)$res[HH\array_key_cast($k)] = $v;
  return $res;
}

<<__EntryPoint>>
function main(): void {
  foreach (vec[false, true] as $mark_legacy) {
    foreach (
      vec[0, FB_SERIALIZE_VARRAY_DARRAY, FB_SERIALIZE_HACK_ARRAYS] as $mode
    ) {
      printf("=== mode=%d, mark_legacy=%d ===\n", $mode, (int)$mark_legacy);
      $vec_original = vec[1, 2, 3];
      $vec_roundtrip = round_trip($vec_original, $mode, $mark_legacy);
      var_dump($vec_roundtrip);
      // test for vec->dict coersion happening as expected
      $vec_expected = $mode === 0 ||
        ($mode === FB_SERIALIZE_HACK_ARRAYS && $mark_legacy)
        ? dict($vec_original)
        : $vec_original;
      invariant($vec_expected === $vec_roundtrip, "");

      $dict_original = dict['1' => 'a', 'x' => 'y'];
      $dict_roundtrip = round_trip(
        $dict_original,
        $mode,
        $mark_legacy,
      );
      var_dump($dict_roundtrip);

      // test for intish cast working as expected
      $dict_expected = $mode != FB_SERIALIZE_HACK_ARRAYS
        ? intish_cast($dict_original)
        : $dict_original;
      invariant($dict_expected === $dict_roundtrip, "");
    }
  }
}
