<?hh

class A { public static function meth() {} }

function getArr() {
  return __hhvm_intrinsics\launder_value(varray[A::class, 'meth']);
}

function getClsMeth() {
  return __hhvm_intrinsics\launder_value(HH\class_meth(A::class, 'meth'));
}

function stringCast($x) { return (string) $x; }
function boolCast($x) { return (bool) $x; }
function doubleCast($x) { return (float)$x; }
function intCast($x) { return (int) $x; }
function arrayCast($x) { return (array)$x; }
function varrayCast($x) { return varray($x); }
function vecCast($x) { return vec($x); }
function darrayCast($x) { return darray($x); }
function dictCast($x) { return dict($x); }
function keysetCast($x) { return keyset($x); }

function test($x): void {
  var_dump(stringCast($x));
  var_dump(boolCast($x));
  var_dump(doubleCast($x));
  var_dump(intCast($x));
  var_dump(arrayCast($x));
  var_dump(varrayCast($x));
  var_dump(vecCast($x));
  var_dump(darrayCast($x));
  var_dump(dictCast($x));
  var_dump(keysetCast($x));
}

<<__EntryPoint>>
function main(): void {
  test(getArr());
  test(getClsMeth());
}
