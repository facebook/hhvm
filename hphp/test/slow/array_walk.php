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
  $a = array(1, 2, 3);
  array_walk(&$a, 'nonref_nodata');
  var_dump($a);

  echo "==== byref_nodata ====\n";
  $a = array(1, 2, 3);
  array_walk(&$a, 'byref_nodata');
  var_dump($a);

  echo "==== nonref_data ====\n";
  $a = array(1, 2, 3);
  array_walk(&$a, 'nonref_data', 'whut');
  var_dump($a);

  echo "==== byref_nodata ====\n";
  $a = array(1, 2, 3);
  array_walk(&$a, 'byref_data', 'whut');
  var_dump($a);

  echo "==== closure ====\n";
  $a = array(1, 2, 3);
  array_walk(&$a, (&$v, $k) ==> {
    var_dump($k, $v);
    $v = 'lol';
  });
  var_dump($a);

  echo "==== hack-array ====\n";
  $x = dict[];
  var_dump(array_walk(&$x, 'noref_nodata'));
  var_dump($x);

  echo "==== non-array ====\n";
  $x = 5.7;
  var_dump(array_walk(&$x, 'noref_nodata'));
  var_dump($x);
}
