<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($v) :mixed{
  echo "Testing: ";
  var_dump($v);

  echo "array_pad (after): ";
  var_dump(array_pad($v, 10, "pad"));
  echo "array_pad (before): ";
  var_dump(array_pad($v, -10, "pad"));
  echo "array_pad (just one): ";
  var_dump(array_pad($v, 1, "pad"));

  echo "array_pop: ";
  $copy_v = $v;
  var_dump(array_pop(inout $copy_v));
  var_dump($copy_v);

  echo "array_product: ";
  var_dump(array_product($v));

  echo "array_push: ";
  $copy_v = $v;
  var_dump(array_push(inout $copy_v, "pushed1", "pushed2", "pushed3"));
  var_dump($copy_v);

  echo "array_search (2): ";
  var_dump(array_search(2, $v));
  echo "array_search (\"not-found\"): ";
  var_dump(array_search("not-found", $v));
  echo "array_search (false): ";
  var_dump(array_search(false, $v));
  echo "array_search (\"2\"): ";
  var_dump(array_search("2", $v));

  echo "array_shift: ";
  $copy_v = $v;
  var_dump(array_shift(inout $copy_v));
  var_dump($copy_v);

  echo "array_sum: ";
  var_dump(array_sum($v));

  echo "array_unshift: ";
  $copy_v = $v;
  var_dump(array_unshift(inout $copy_v, "prepend1", "prepend2"));
  var_dump($copy_v);

  echo "array_values: ";
  var_dump(array_values($v));

  echo "count: ";
  var_dump(count($v));

  echo "empty: ";
  var_dump(!($v ?? false));

  echo "in_array (3): ";
  var_dump(in_array(3, $v));
  echo "in_array (\"not-found\"): ";
  var_dump(in_array("not-found", $v));
  echo "in_array (false): ";
  var_dump(in_array(false, $v));
  echo "in_array (\"3\"): ";
  var_dump(in_array("3", $v));

  echo "sort: ";
  $copy_v = $v;
  var_dump(sort(inout $copy_v));
  var_dump($copy_v);

  echo "rsort: ";
  $copy_v = $v;
  var_dump(rsort(inout $copy_v));
  var_dump($copy_v);

  echo "asort: ";
  $copy_v = $v;
  var_dump(asort(inout $copy_v));
  var_dump($copy_v);

  echo "arsort: ";
  $copy_v = $v;
  var_dump(arsort(inout $copy_v));
  var_dump($copy_v);

  echo "ksort: ";
  $copy_v = $v;
  var_dump(ksort(inout $copy_v));
  var_dump($copy_v);

  echo "krsort: ";
  $copy_v = $v;
  var_dump(krsort(inout $copy_v));
  var_dump($copy_v);

  echo "array_slice (0): ";
  var_dump(array_slice($v, 0));

  echo "array_slice (1): ";
  var_dump(array_slice($v, 1));
}
<<__EntryPoint>> function main_entry(): void {
main(vec[]);
main(vec[1, 2, 3, 4, 5]);
main(vec[5, 4, 3, 2, 1]);
main(vec["a", "b", "c"]);
main(vec["b", "a", "a", "b"]);
main(vec[100]);
}
