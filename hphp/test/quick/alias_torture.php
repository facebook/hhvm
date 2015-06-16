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

function main1() {
  global $b;
  $GLOBALS['b'] = 123; // via SetG
  var_dump($b);

  $b = 123;
  $GLOBALS['b'] .= "moot"; // via SetOpG
  var_dump($b);

  $b = 123;
  $c = $GLOBALS;
  $c['b'] = "hoof"; // via SetM
  var_dump($b);
}
main1();

class A {
  function __destruct() {
    global $x, $y, $z;
    $x = "foo";
    $y = "bar";
    $z = "baz";
  }
}

function main2() {
// SetH calling __destruct()
  global $a, $b, $c;
  global $x, $y, $z;
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
}

main2();

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

// Exercise aliasing across re-entry.
mt_srand(0);
function randphpwork($r) {
  $a = array();
  switch($r % 4) {
  case 0: /* int */ return mt_rand(0, 256);
  case 1: /* str */ return randstr();
  case 2: /* obj */ return new C($a = randarr());
  case 3: /* arr */ return randarr();
  }
  return null; // not reached
}

function randphp() {
  static $depth;
  $r = mt_rand(0, 4);
  if ($depth > 2) $r = $r & 1; // scalars
  $depth++;
  $r = $r % 4;
  $ret = randphpwork($r);
  $depth--;
  return $ret;
}

function randchar() {
  return chr(ord('A') + (mt_rand(0, 26)));
}

function randstr() {
  $ret = "    ";
  $ret[0] = randchar();
  $ret[1] = randchar();
  $ret[2] = randchar();
  $ret[3] = randchar();
  return $ret;
}

function randarr() {
  $ret = array();
  $ret[] = randstr();
  $ret[] = randstr();
  $ret[] = randstr();
  $ret[] = randstr();
  return $ret;
}

class C {
  public $aliases;
  public function __construct($aliases) {
    $this->aliases = $aliases;
  }
  public function __destruct() {
    foreach($this->aliases as &$arf) {
      $arf = randphp();
    }
  }
}

function tmpobj(&$aliases) {
  return new C($aliases);
}

function main3() {
  // Get some locals.
  $a = mt_rand(0, 10);
  $b = randstr();
  $c = randarr();
  $str = randstr();

  // Alias them in $aliases.
  $aliases = array();
  $aliases[]= &$a;
  $aliases[]= &$b;
  $aliases[]= &$c;

  // Wrap them up in an array and leak them into C().
  for ($i = 0; $i < 10; $i++) {
    echo "$i <\n";
    // Get some locals.
    $a = $i;
    $b = randstr();
    $c = randarr();
    // Read/write them while implicitly mutating them through the temporary
    // object's destructor.
    $unused = (tmpobj($aliases) === tmpobj($aliases)) === (($a === $b) === $c);
    echo " --------> \n";
    // ...and use them again.
    var_dump($a, $b, $c);
    echo ">\n";
  }
}

main3();
