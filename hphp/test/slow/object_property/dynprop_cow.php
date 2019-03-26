<?hh // decl

function test_read() {
  echo "---- read ----\n";
  $y = 'b';
  $x = array('a' => &$y, 'c' => 'd');
  $z = (object)$x;
  var_dump($z->c);
  unset($y);
  var_dump($x);
}

function test_dim_read() {
  echo "---- dim read ----\n";
  $y = 'b';
  $x = array('a' => &$y, 'c' => array('d'));
  $z = (object)$x;
  var_dump($z->c[0]);
  unset($y);
  var_dump($x);
}

function test_read_quiet() {
  echo "---- read quiet ----\n";
  $y = 'b';
  $x = array('a' => &$y, 'c' => 'd');
  $z = (object)$x;
  var_dump($z?->c);
  unset($y);
  var_dump($x);
}

function test_dim_read_quiet() {
  echo "---- dim read quiet----\n";
  $y = 'b';
  $x = array('a' => &$y, 'c' => array('d'));
  $z = (object)$x;
  var_dump($z?->c[0]);
  unset($y);
  var_dump($x);
}

function test_set() {
  echo "---- set ----\n";
  $y = 'b';
  $x = array('a' => &$y, 'c' => 'd');
  $z = (object)$x;
  $z->c = 'e';
  unset($y);
  var_dump($x);
}

function test_dim_set() {
  echo "---- dim set ----\n";
  $y = 'b';
  $x = array('a' => &$y, 'c' => array('d'));
  $z = (object)$x;
  $z->c[0] = 'e';
  unset($y);
  var_dump($x);
}

function test_unset() {
  echo "---- unset ----\n";
  $y = 'b';
  $x = array('a' => &$y, 'c' => 'd');
  $z = (object)$x;
  unset($z->c);
  unset($y);
  var_dump($x);
}

function test_dim_unset() {
  echo "---- dim unset ----\n";
  $y = 'b';
  $x = array('a' => &$y, 'c' => array('d'));
  $z = (object)$x;
  unset($z->c[0]);
  unset($y);
  var_dump($x);
}


<<__EntryPoint>>
function main_dynprop_cow() {
test_read();
test_dim_read();
test_read_quiet();
test_dim_read_quiet();
test_set();
test_dim_set();
test_unset();
test_dim_unset();
}
