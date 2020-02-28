<?hh

class C { function heh() { echo "heh\n"; } }
function foo() { return varray[]; }
function some_int() { return mt_rand() ? 1 : 2; }
function bar() {
  $x = foo();
  $val = some_int();
  $x[$val] = new C;

  HhbbcArray041::$g = $val;
  return $x;
}

<<__EntryPoint>>
function main_array_041() {

  $y = bar();
  $l = (int)HhbbcArray041::$g;
  $y = $y[$l];
  $y->heh();
}


abstract final class HhbbcArray041 {
  public static $g;
}
