<?hh // strict

function test_compare(DateTime $d): bool {
  $new_datetime = new DateTime();
  return $d < $new_datetime;
}

function test_compare_generics<T as DateTime>(T $d1, DateTime $d2): bool {
  return $d1 > $d2;
}
