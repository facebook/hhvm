<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($k) :mixed{
  echo "Testing: ";
  var_dump($k);

  echo "array_pad (after): ";
  var_dump(array_pad($k, 10, "pad"));
  echo "array_pad (before): ";
  var_dump(array_pad($k, -10, "pad"));
  echo "array_pad (just one): ";
  var_dump(array_pad($k, 1, "pad"));

  echo "array_pop: ";
  $copy_k = $k;
  var_dump(array_pop(inout $copy_k));
  var_dump($copy_k);

  echo "array_product: ";
  var_dump(array_product($k));

  echo "array_push: ";
  $copy_k = $k;
  var_dump(array_push(inout $copy_k, "pushed1", "pushed2", "pushed3"));
  var_dump($copy_k);

  echo "array_search (2): ";
  var_dump(array_search(2, $k));
  echo "array_search (\"not-found\"): ";
  var_dump(array_search("not-found", $k));
  echo "array_search (false): ";
  var_dump(array_search(false, $k));
  echo "array_search (\"2\"): ";
  var_dump(array_search("2", $k));

  echo "array_shift: ";
  $copy_k = $k;
  var_dump(array_shift(inout $copy_k));
  var_dump($copy_k);

  echo "array_sum: ";
  var_dump(array_sum($k));

  echo "array_unshift: ";
  $copy_k = $k;
  var_dump(array_unshift(inout $copy_k, "prepend1", "prepend2"));
  var_dump($copy_k);

  echo "array_values: ";
  var_dump(array_values($k));

  echo "count: ";
  var_dump(count($k));

  echo "empty: ";
  var_dump(!($k ?? false));

  echo "in_array (3): ";
  var_dump(in_array(3, $k));
  echo "in_array (\"not-found\"): ";
  var_dump(in_array("not-found", $k));
  echo "in_array (false): ";
  var_dump(in_array(false, $k));
  echo "in_array (\"3\"): ";
  var_dump(in_array("3", $k));

  echo "asort: ";
  $copy_k = $k;
  var_dump(asort(inout $copy_k));
  var_dump($copy_k);

  echo "arsort: ";
  $copy_k = $k;
  var_dump(arsort(inout $copy_k));
  var_dump($copy_k);

  echo "ksort: ";
  $copy_k = $k;
  var_dump(ksort(inout $copy_k));
  var_dump($copy_k);

  echo "krsort: ";
  $copy_k = $k;
  var_dump(krsort(inout $copy_k));
  var_dump($copy_k);

  echo "array_slice (0): ";
  var_dump(array_slice($k, 0));

  echo "array_slice (1): ";
  var_dump(array_slice($k, 1));
}
<<__EntryPoint>> function main_entry(): void {
main(keyset[]);
main(keyset[1, 2, 3, 4, 5]);
main(keyset[5, 4, 3, 2, 1]);
main(keyset["a", "b", "c"]);
main(keyset["b", "a", "a", "b"]);
main(keyset[100]);
}
