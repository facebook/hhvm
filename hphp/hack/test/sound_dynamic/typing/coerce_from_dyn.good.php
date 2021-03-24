<?hh

function f_bool(bool $i) : void {}
function f_int(int $i) : void {}
function f_float(float $i) : void {}
function f_num(num $i) : void {}
function f_string(string $i) : void {}
function f_arraykey(arraykey $i) : void {}
function f_null(null $i) : void {}
function f_nonnull(nonnull $i) : void {}

function test_prim(dynamic $d) : void {
  f_bool($d);
  f_int($d);
  f_float($d);
  f_num($d);
  f_string($d);
  f_bool($d);
  f_arraykey($d);
  f_null($d);
  f_nonnull($d);
}

<<__SoundDynamicCallable>>
class C {}

function f_c(C $c) : void {}

function test_class(dynamic $d) : void {
  f_c($d);
}

class D<reify T> {}

function f_reify(D<D<int>> $d) : void {}

function test_reify(dynamic $d) : void {
  f_reify($d);
}

enum E : int {
  CONST = 1;
}

function f_enum(E $e) : void {}

function test_enum(dynamic $d) : void {
  f_enum($d);
}

function f_option(?int $e) : void {}

function test_option(dynamic $d) : void {
  f_option($d);
}

function test_inter(dynamic $d) : void {
  if ($d is int) {
    f_int($d);
    f_string($d);
  }
}

function test_union(dynamic $d, bool $b) : void {
  if ($b) {
    $x = $d;
  } else {
    $x = 1;
  }
  f_int($x);
}
