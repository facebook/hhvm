<?hh


<<__EntryPoint>>
function main_804() :mixed{
try {
  $v = new Vector();
  $v->containsKey(1);
  try {
    $v->containsKey('foo');
  }
 catch (Exception $e) {
    echo 'A';
  }
  try {
    $v->containsKey(1.0);
  }
 catch (Exception $e) {
    echo 'B';
  }
  $methods = Vector::fromArray(vec['containsKey','remove']);
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
    echo "\n";
  }
}
 catch (Exception $e) {
  echo "Test failed\n";
}
}
