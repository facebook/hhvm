<?hh

function error_handler($errno, $errstr) {
  echo $errstr . "\n";
}

class A {
  public $x = 123;
  <<__SoftLateInit>> public $y;
  public $z = 'abc';
}

<<__EntryPoint>> function main(): void {
  set_error_handler(fun('error_handler'));
  HH\set_soft_late_init_default(123);

  $a = new A();
  $b = new A();
  var_dump($a == $b);
  var_dump($a == $b);
  var_dump($a);
  var_dump($b);

  $a = new A();
  $b = new A();
  $a->y = 'abc';
  var_dump($a == $b);
  var_dump($a == $b);
  var_dump($a);
  var_dump($b);

  $a = new A();
  $b = new A();
  $b->y = 'abc';
  var_dump($a == $b);
  var_dump($a == $b);
  var_dump($a);
  var_dump($b);

  $a = new A();
  $b = new A();
  $a->y = 'abc';
  $b->y = 'abc';
  var_dump($a == $b);
  var_dump($a == $b);
  var_dump($a);
  var_dump($b);

  $a = new A();
  $b = new A();
  var_dump($a <=> $b);
  var_dump($a <=> $b);
  var_dump($a);
  var_dump($b);

  $a = new A();
  $b = new A();
  $a->y = 'abc';
  var_dump($a <=> $b);
  var_dump($a <=> $b);
  var_dump($a);
  var_dump($b);

  $a = new A();
  $b = new A();
  $b->y = 'abc';
  var_dump($a <=> $b);
  var_dump($a <=> $b);
  var_dump($a);
  var_dump($b);

  $a = new A();
  $b = new A();
  $a->y = 'abc';
  $b->y = 'abc';
  var_dump($a <=> $b);
  var_dump($a <=> $b);
  var_dump($a);
  var_dump($b);
}
