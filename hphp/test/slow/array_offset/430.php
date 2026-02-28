<?hh

class A {
  const i1= -1;
  const i2= -2;
  public static $s = -4;
}
class B {
  public static $s = -5;
}

<<__EntryPoint>>
function main_430() :mixed{
;
;
$attr=dict[];
$attr[A::i1]='abc';
$attr[A::i2]='def';
$attr[-3]='ghi';
$attr[A::$s]='jkl';
$attr[B::$s]='mno';
var_dump($attr);
}
