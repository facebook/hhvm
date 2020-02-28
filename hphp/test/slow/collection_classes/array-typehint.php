<?hh
function handler($errno, $errmsg) {
  if ($errno === E_RECOVERABLE_ERROR) {
    throw new Exception("Type constraint failed");
  } else if ($errno === E_WARNING) {
    echo "Triggered E_WARNING with message \"$errmsg\"\n";
  } else if ($errno === E_NOTICE) {
    echo "Triggered E_NOTICE with message \"$errmsg\"\n";
  } else {
    return false;
  }
}

function helper($x) {
  var_dump($x);
  if ($x is Map) {
    $x['z'] = 4;
    var_dump($x);
  } else if (is_array($x) || $x is Collection) {
    $x[] = 4;
    var_dump($x);
  }
}
function helper_ref(inout $x) {
  var_dump($x);
  if ($x is Map) {
    $x['z'] = 4;
    var_dump($x);
  } else if (is_array($x) || $x is Collection) {
    $x[] = 4;
    var_dump($x);
  }
}

function f1(array $x) { helper($x); }
function f2(?array $x) { helper($x); }
function f3(@array $x) { helper($x); }
function f4(@?array $x) { helper($x); }
function f5(inout array $x) { helper_ref(inout $x); }
function f6(inout ?array $x) { helper_ref(inout $x); }
function f7(inout @array $x) { helper_ref(inout $x); }
function f8(inout @?array $x) { helper_ref(inout $x); }

function main() {
  $containers = Map {
    'array' => varray[1, 2, 3],
    'Vector' => Vector {1, 2, 3},
    'Map' => Map {'a' => 1, 'b' => 2, 'c' => 3},
    'Set' => Set {1, 2, 3},
    'stdClass' => new stdClass()
  };
  $first = true;
  foreach ($containers as $name => $c) {
    echo "==== $name ====\n";
    for ($i = 1; $i <= 8; ++$i) {
      $fn = 'f' . $i;
      echo "$fn:\n";
      $x = ($c is Collection) ? clone $c : $c;
      try {
        $i <= 4 ? $fn($x) : $fn(inout $x);
      } catch (Exception $e) {
        echo $fn . "() threw an exception\n";
      }
      unset($x);
    }
  }
}

<<__EntryPoint>>
function main_array_typehint() {
error_reporting(-1);
set_error_handler(fun('handler'));
main();
}
