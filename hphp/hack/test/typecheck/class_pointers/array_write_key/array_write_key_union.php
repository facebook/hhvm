<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class A {
  public static function meth(): void {}
}

class B {
  public static function meth(): void {}
}

function union_write(class<A> $a, class<B> $b, bool $cond): void {
  $key = $cond ? $a : $b;
  $d = dict[];
  $d[$key] = 1;
  foreach ($d as $key => $_) {
    $key::meth();
  }
}

function union_keyset_append(
  class<A> $a,
  class<B> $b,
  bool $cond,
): void {
  $value = $cond ? $a : $b;
  $keyset = keyset[];
  $keyset[] = $value;
  foreach ($keyset as $value) {
    $value::meth();
  }
}

function union_set_append(
  class<A> $a,
  class<B> $b,
  bool $cond,
): void {
  $value = $cond ? $a : $b;
  $set = Set {};
  $set[] = $value;
  foreach ($set as $value) {
    $value::meth();
  }
}

function union_vec_append_unchanged(
  class<A> $a,
  class<B> $b,
  bool $cond,
): void {
  $value = $cond ? $a : $b;
  $vec = vec[];
  $vec[] = $value;
  foreach ($vec as $value) {
    $value::meth();
  }
}
