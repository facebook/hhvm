<?hh
function handler($errno, $errmsg) :mixed{
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

function helper($x) :mixed{
  var_dump($x);
  if ($x is Map) {
    $x['z'] = 4;
    var_dump($x);
  } else if (is_array($x) || $x is Collection) {
    $x[] = 4;
    var_dump($x);
  }
}
function helper_ref(inout $x) :mixed{
  var_dump($x);
  if ($x is Map) {
    $x['z'] = 4;
    var_dump($x);
  } else if (is_array($x) || $x is Collection) {
    $x[] = 4;
    var_dump($x);
  }
}

function f1(varray $x) :mixed{ helper($x); }
function f2(?varray $x) :mixed{ helper($x); }
function f3(<<__Soft>> varray $x) :mixed{ helper($x); }
function f4(<<__Soft>> ?varray $x) :mixed{ helper($x); }
function f5(inout varray $x) :mixed{ helper_ref(inout $x); }
function f6(inout ?varray $x) :mixed{ helper_ref(inout $x); }
function f7(<<__Soft>> inout varray $x) :mixed{ helper_ref(inout $x); }
function f8(<<__Soft>> inout ?varray $x) :mixed{ helper_ref(inout $x); }

function main() :mixed{
  $containers = Map {
    'array' => vec[1, 2, 3],
    'Vector' => Vector {1, 2, 3},
    'Map' => Map {'a' => 1, 'b' => 2, 'c' => 3},
    'Set' => Set {1, 2, 3},
    'stdClass' => new stdClass()
  };
  $first = true;

  $funcs = vec[
    tuple("f1", f1<>),
    tuple("f2", f2<>),
    tuple("f3", f3<>),
    tuple("f4", f4<>),
    tuple("f5", f5<>),
    tuple("f6", f6<>),
    tuple("f7", f7<>),
    tuple("f8", f8<>),
  ];

  foreach ($containers as $name => $c) {
    echo "==== $name ====\n";
    foreach ($funcs as $i => list($name, $fn)) {
      echo "$name:\n";
      $x = ($c is Collection) ? clone $c : $c;
      try {
        $i <= 3 ? $fn($x) : $fn(inout $x);
      } catch (Exception $e) {
        echo $name . "() threw an exception\n";
      }
      unset($x);
    }
  }
}

<<__EntryPoint>>
function main_array_typehint() :mixed{
error_reporting(-1);
set_error_handler(handler<>);
main();
}
