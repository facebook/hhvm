<?hh

try {
  $v = new Vector();
  $v->contains(1);
  try {
    $v->contains('foo');
  }
 catch (Exception $e) {
    echo 'A';
  }
  try {
    $v->contains(1.0);
  }
 catch (Exception $e) {
    echo 'B';
  }
  $methods = Vector::fromArray(array('contains','remove'));
  foreach ($methods as $method) {
    $m = new Map();
    $m->$method(1);
    $m->$method('foo');
    try {
      $m->$method(1.0);
    }
 catch (Exception $e) {
      echo 'C';
    }
    $s = new StableMap();
    $s->$method(1);
    $s->$method('foo');
    try {
      $s->$method(1.0);
    }
 catch (Exception $e) {
      echo 'D';
    }
    echo "\n";
  }
}
 catch (Exception $e) {
  echo "Test failed\n";
}
