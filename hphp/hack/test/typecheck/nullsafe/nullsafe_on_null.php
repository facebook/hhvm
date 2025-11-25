<?hh

class A {
  public int $value = 0;
}

function nullsafe_on_null(?A $a): void {
  $a?->value; // OK
  if ($a is null) {
    $a?->value; // Not OK since $a is known to be null
  }
}
