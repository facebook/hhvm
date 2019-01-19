<?hh

function nonref_nodata($v, $k) {
  var_dump($k, $v);
  $v = 'wat';
}

function byref_nodata(&$v, $k) {
  var_dump($k, $v);
  $v = 'wat';
}

function nonref_data($v, $k, $data) {
  var_dump($k, $v, $data);
  $v = $data;
}

function byref_data(&$v, $k, $data) {
  var_dump($k, $v, $data);
  $v = $data;
}

<<__EntryPoint>>
function main() {
  echo "==== nonref_nodata ====\n";
  $a = array(1, 2, array(3, 4, 5));
  array_walk_recursive(&$a, 'nonref_nodata');
  var_dump($a);

  echo "==== byref_nodata ====\n";
  $a = array(1, 2, array(3, 4, 5));
  array_walk_recursive(&$a, 'byref_nodata');
  var_dump($a);

  echo "==== nonref_data ====\n";
  $a = array(1, 2, array(3, 4, 5));
  array_walk_recursive(&$a, 'nonref_nodata', 'whut');
  var_dump($a);

  echo "==== byref_data ====\n";
  $a = array(1, 2, array(3, 4, 5));
  array_walk_recursive(&$a, 'byref_nodata', 'whut');
  var_dump($a);

  echo "==== closure ====\n";
  $a = array(1, 2, array(3, 4, 5));
  array_walk_recursive(&$a, (&$v, $k) ==> {
    var_dump($k, $v);
    $v = 'lol';
  });
  var_dump($a);

  echo "==== hack array ====\n";
  $x = vec[1, 2, 3];
  var_dump(array_walk_recursive(&$x, 'byref_nodata'));
  var_dump($x);

  echo "==== nested hack array ====\n";
  $x = array(1, array(2, 3), vec[4, 5]);
  var_dump(array_walk_recursive(&$x, 'byref_nodata'));
  var_dump($x);

  echo "==== non-array ====\n";
  $x = 4.6;
  var_dump(array_walk_recursive(&$x, 'byref_nodata'));
  var_dump($x);
}
