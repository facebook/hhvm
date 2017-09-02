<?hh

function test_read() {
  echo "---- read ----\n";
  $x = array('a' => 'b', 'c' => 'd');
  $y =& $x['a'];
  $z = (object)$x;
  var_dump($z->c);
  unset($y);
  var_dump($x);
}

function test_dim_read() {
  echo "---- dim read ----\n";
  $x = array('a' => 'b', 'c' => array('d'));
  $y =& $x['a'];
  $z = (object)$x;
  var_dump($z->c[0]);
  unset($y);
  var_dump($x);
}

function test_read_quiet() {
  echo "---- read quiet ----\n";
  $x = array('a' => 'b', 'c' => 'd');
  $y =& $x['a'];
  $z = (object)$x;
  var_dump($z?->c);
  unset($y);
  var_dump($x);
}

function test_dim_read_quiet() {
  echo "---- dim read quiet----\n";
  $x = array('a' => 'b', 'c' => array('d'));
  $y =& $x['a'];
  $z = (object)$x;
  var_dump($z?->c[0]);
  unset($y);
  var_dump($x);
}

function test_set() {
  echo "---- set ----\n";
  $x = array('a' => 'b', 'c' => 'd');
  $y =& $x['a'];
  $z = (object)$x;
  $z->c = 'e';
  unset($y);
  var_dump($x);
}

function test_dim_set() {
  echo "---- dim set ----\n";
  $x = array('a' => 'b', 'c' => array('d'));
  $y =& $x['a'];
  $z = (object)$x;
  $z->c[0] = 'e';
  unset($y);
  var_dump($x);
}

function test_unset() {
  echo "---- unset ----\n";
  $x = array('a' => 'b', 'c' => 'd');
  $y =& $x['a'];
  $z = (object)$x;
  unset($z->c);
  unset($y);
  var_dump($x);
}

function test_dim_unset() {
  echo "---- dim unset ----\n";
  $x = array('a' => 'b', 'c' => array('d'));
  $y =& $x['a'];
  $z = (object)$x;
  unset($z->c[0]);
  unset($y);
  var_dump($x);
}

test_read();
test_dim_read();
test_read_quiet();
test_dim_read_quiet();
test_set();
test_dim_set();
test_unset();
test_dim_unset();
