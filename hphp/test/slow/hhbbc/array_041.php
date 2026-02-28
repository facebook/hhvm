<?hh

class C { function heh() :mixed{ echo "heh\n"; } }
function foo() :mixed{ return dict[]; }
function some_int() :mixed{ return mt_rand() ? 1 : 2; }
function bar() :mixed{
  $x = foo();
  $val = some_int();
  $x[$val] = new C;

  HhbbcArray041::$g = $val;
  return $x;
}

<<__EntryPoint>>
function main_array_041() :mixed{

  $y = bar();
  $l = (int)HhbbcArray041::$g;
  $y = $y[$l];
  $y->heh();
}


abstract final class HhbbcArray041 {
  public static $g;
}
