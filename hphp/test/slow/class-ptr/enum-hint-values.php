<?hh

<<file:__EnableUnstableFeatures('class_type')>>

enum E : int {
  A = 1;
  B = 2;
}

enum F : string {
  S = "s";
  T = "t";
}

function test(enum<mixed> $m): void {
  var_dump($m::getValues());
}
// test/slow/ext_enum/enumname.php
type enumname<T> = HH\enumname<T>;
function test2(enumname<mixed> $m): void {
  var_dump($m::getValues());
}

<<__EntryPoint>>
function main(): void {
  $le = E::class;
  $e = HH\classname_to_class($le);

  test($le);
  test2($e);

  $lf = F::class;
  $f = HH\classname_to_class($lf);

  test($lf);
  test2($f);
}
