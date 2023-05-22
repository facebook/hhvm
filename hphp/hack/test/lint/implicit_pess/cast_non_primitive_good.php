//// a.php

newtype SomeNewtype = int;

//// b.php
<?hh

function cast_non_primitive_good1(): void {
  (string) 7;
}

function cast_non_primitive_good2(?string $bar): void {
  (int) $bar;
}

function cast_non_primitive_good3(bool $test): void {
  if ($test) {
    $x = 'test';
  } else {
    $x = false;
  }
  (string) $x;
}

enum E : int {
  A = 42;
}

function cast_non_primitive_good4(E $e): void {
  (string) $e;
  (int) E::A;
}

function cast_non_primitive_good5(): void {
  (string) idx(dict[], 'banana');
}

function cast_non_primitive_good6(SomeNewtype $x): void {
  (string) $x;
}

function cast_non_primitive_good7(mixed $test): void {
  (string) $test;
}

function cast_non_primitive_good8(mixed $test): void {
  (string) idx($test as KeyedContainer<_, _>, 'banana');
}

function cast_non_primitive_good9(
  bool $test1,
  bool $test2,
  bool $test3,
): void {
  if ($test1) {
    $a = 'a';
  } else {
    $a = false;
  }

  if ($test2) {
    $b = 12;
  } else {
    $b = 12.4;
  }

  if ($test3) {
    $c = $a;
  } else {
    $c = $b;
  }
  (int)$c;
}

function cast_non_primitive_good10_helper(): dict<string, mixed> {
  return dict[];
}

function cast_non_primitive_good10(): ?string {
  return (string) idx(cast_non_primitive_good10_helper(), 'foo');
}

function cast_non_primitive_good11(
  HH\FormatString<Str\SprintfFormat> $x,
): void {
  (string) $x;
}

function cast_non_primitive_good12(
  vec<string> $vec,
  dict<string, string> $dict,
  keyset<string> $keyset,
  vec<string> $varray,
  dict<string, string> $darray,
  vec_or_dict<string> $varray_or_darray,
): void {
  (bool) $vec;
  (bool) $dict;
  (bool) $keyset;
  (bool) $varray;
  (bool) $darray;
  (bool) $varray_or_darray;
}
