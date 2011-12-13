<?

function foo() {
  global $a;
  $a = 123;
  $GLOBALS['a'] = "the"; // violate type inference via SetG
  var_dump($a);
  
  $a = 123;
  $GLOBALS['a'] .= "duke"; // via SetOpG
  var_dump($a);

  $a = 123;
  $b = $GLOBALS;
  $b['a'] = "of"; // via SetM
  var_dump($a);
}

foo();

$GLOBALS['b'] = 123; // via SetG
var_dump($b);

$b = 123;
$GLOBALS['b'] .= "moot"; // via SetOpG
var_dump($b);

$b = 123;
$c = $GLOBALS;
$c['b'] = "hoof"; // via SetM
var_dump($b);

class A {
  function __destruct() {
    global $x, $y, $z;
    $x = "foo";
    $y = "bar";
    $z = "baz";
  }
}

// SetH calling __destruct()

$x = 123;
$a = new A;
$a = null;
var_dump($x);

$y = 123;
$b = new A;
$c = &$b;
$b = null;
var_dump($y);

$z = 123;
$d = array();
$d[] = new A;
$d = null;
var_dump($z);

class B {
  function __construct(&$x) {
    $this->y = &$x;
  }
  function __destruct() {
    $this->y = "foo";
  }
}

function bar() {
  $x = 123;
  $a = new B($x);
  $a = null;
  var_dump($x);

  $y = 123;
  $b = new B($y);
  $c = &$b;
  $b = null;
  var_dump($y);

  $z = 123;
  $d = array();
  $d[] = new B($z);
  $d = null;
  var_dump($z);
}

bar();
