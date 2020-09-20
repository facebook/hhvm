<?hh

class X {
}
function f($a0,
           $a1,
           $a2,
           $a3 = null,
           $a4 = null,
           $a5 = null) {
  $r0 = $a0 ?: 0;
  $r1 = $a1 ?: 0.0;
  $r2 = $a2 ?: false;
  $r3 = $a3 ?: '';

  $r4 = $a4 ?: varray[];
  $r5 = $a5 ?: new X;
  return varray[
    $r0, $r1, $r2,
    $r3, $r4, $r5];
}

<<__EntryPoint>>
function main_1737() {
var_dump(f(0, 0.0, false, null, null, null));
var_dump(f(1, 1.0, true, 'hello', varray[0, 1], new X));
}
