<?hh

class A {
  const i1= -1;
  const i2= -2;
  static $s = -4;
}
class B {
  static $s = -5;
}

<<__EntryPoint>>
function main_430() {
;
;
$attr=darray[];
$attr[A::i1]='abc';
$attr[A::i2]='def';
$attr[-3]='ghi';
$attr[A::$s]='jkl';
$attr[B::$s]='mno';
var_dump($attr);
}
