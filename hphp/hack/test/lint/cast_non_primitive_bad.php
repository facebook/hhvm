<?hh

final class C {}
final class D {}
final class E {}
final class F {}

function cast_non_primitive_bad1(): void {
  (int) (Vector {});
}

function cast_non_primitive_bad2(): void {
  (int) (vec[]);
}

function cast_non_primitive_bad3(): void {
  (int) (new C());
}

function test_non_primitive_bad4(bool $test): void {
  if ($test) {
    $x = 'test';
  } else {
    $x = vec[];
  }
  (int) $x;
}

function test_non_primitive_bad5(bool $test): void {
  if ($test) {
    $x = 'test';
  } else {
    $x = new C();
  }
  (int) $x;
}

function cast_non_primitive_bad6(bool $test): void {
  if ($test) {
    $x = vec[];
  } else {
    $x = new C();
  }
  (int) $x;
}

function cast_non_primitive_bad7(Stringish $test): void {
  (int) $test;
}

function cast_non_primitive_bad8(
  bool $test1,
  bool $test2,
  bool $test3,
): void {
  if ($test1) {
    $a = new C();
  } else {
    $a = new D();
  }

  if ($test2) {
    $b = new E();
  } else {
    $b = new F();
  }

  if ($test3) {
    $c = $a;
  } else {
    $c = $b;
  }
  (int) $c;
}

function cast_non_primitive_bad9(
  vec<string> $vec,
  dict<string, string> $dict,
  keyset<string> $keyset,
  vec<string> $varray,
  dict<string, string> $darray,
  vec_or_dict<string> $varray_or_darray,
): void {
  (int)$vec;
  (int)$dict;
  (int)$keyset;
  (int)$varray;
  (int)$darray;
  (int)$varray_or_darray;
}
