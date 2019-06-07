<?hh

function __autoload($x) {

  $GLOBALS['y'] = new stdclass;
  if (mt_rand()) {
    include 'static_inits_2-1.inc';
  } else {
    include 'static_inits_2-2.inc';
  }
}

class A {
  public static $foo = Zoo::Bar;
  function k() {
    echo "ok\n";
  }
}

$z = new A;
$z->k();
var_dump($z);
var_dump($y);
