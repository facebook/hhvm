<?hh // strict

function test_compare(DateTimeImmutable $d): bool {
  $new_datetime = new DateTimeImmutable();
  return $d < $new_datetime;
}

function test_compare_generics<T as DateTimeImmutable>(
  T $d1,
  DateTimeImmutable $d2,
): bool {
  return $d1 > $d2;
}

function test_compare_mutable_and_immutable(
  DateTime $d1,
  DateTimeImmutable $d2,
): bool {
  return $d1 < $d2;
}
