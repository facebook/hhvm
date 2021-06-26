<?hh

class A {
  <<__DynamicallyCallable>> function itest($a, $b) {
    var_dump($a, $b);
  }
  <<__DynamicallyCallable>> static function stest($a, $b) {
    var_dump($a, $b);
  }
}

<<__EntryPoint>>
function main_1207() {
  $i = 'itest';
  $s = 'stest';
  $o = new A();
  $ar = varray[0,1];
  $st = 'abc';
  $o->$i($ar[0], $st[0]);
  A::$s($ar[1], $st[1]);
}
