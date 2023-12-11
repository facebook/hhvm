<?hh

class A {
  <<__DynamicallyCallable>> function itest($a, $b) :mixed{
    var_dump($a, $b);
  }
  <<__DynamicallyCallable>> static function stest($a, $b) :mixed{
    var_dump($a, $b);
  }
}

<<__EntryPoint>>
function main_1207() :mixed{
  $i = 'itest';
  $s = 'stest';
  $o = new A();
  $ar = vec[0,1];
  $st = 'abc';
  $o->$i($ar[0], $st[0]);
  A::$s($ar[1], $st[1]);
}
