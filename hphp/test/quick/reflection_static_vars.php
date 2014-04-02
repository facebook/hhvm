<?php
class Bar {
  const X = 'x';
  static function foo() {
    static $a;
    static $b = 3;
    static $c = Bar::X;
    static $d = array('4' => array(5));
    $b++;
    echo "Bar::foo():\n";
    echo "a = ";var_dump($a);
    echo "b = ";var_dump($b);
    echo "c = ";var_dump($c);
    echo "d = ";var_dump($d);
  }
}

function foo() {
  static $a;
  static $b = 3;
  static $c = Bar::X;
  static $d = array('4' => array(5));
  $b++;
  echo "foo():\n";
  echo "a = ";var_dump($a);
  echo "b = ";var_dump($b);
  echo "c = ";var_dump($c);
  echo "d = ";var_dump($d);
}

$foo = function() {
  static $a;
  static $b = 3;
  // static $c = Bar::X;
  static $d = array('4' => array(5));
  $b++;
  echo "\$foo():\n";
  echo "a = ";var_dump($a);
  echo "b = ";var_dump($b);
  // echo "c = ";var_dump($c);
  echo "d = ";var_dump($d);
};

echo "---- ReflectionFunction ----\n";
$rf = new ReflectionFunction('foo');
echo "ReflectionFunction(1):"; var_dump($rf->getStaticVariables());
foo();
echo "ReflectionFunction(2):"; var_dump($rf->getStaticVariables());
foo();
$rf = new ReflectionFunction('foo');
echo "ReflectionFunction(3):"; var_dump($rf->getStaticVariables());

echo "---- ReflectionMethod ----\n";
$rf = new ReflectionMethod('Bar', 'foo');
echo "ReflectionMethod(1):"; var_dump($rf->getStaticVariables());
Bar::foo();
echo "ReflectionMethod(2):"; var_dump($rf->getStaticVariables());
Bar::foo();
$rf = new ReflectionFunction('foo');
echo "ReflectionMethod(3):"; var_dump($rf->getStaticVariables());

echo "---- ReflectionClosure ----\n";
$rf = new ReflectionFunction($foo);
echo "ReflectionFunction(1-closure):"; var_dump($rf->getStaticVariables());
$foo();
echo "ReflectionFunction(2-closure):"; var_dump($rf->getStaticVariables());
$foo();
$rf = new ReflectionFunction($foo);
echo "ReflectionFunction(3-closure):"; var_dump($rf->getStaticVariables());
