<?hh

<<__Memoize(#MakeICInaccessible)>>
function memoNoArgOK() :mixed{
  return 1;
}

<<__Memoize(#MakeICInaccessible)>>
function memoOK($x) :mixed{
  return $x;
}

<<__Memoize(#MakeICInaccessible)>>
function memoNoArgFail() :mixed{
  echo ClassContext::getContext()->name() . "\n";
  return 1;
}

<<__Memoize(#MakeICInaccessible)>>
function memoFail($x) :mixed{
  echo ClassContext::getContext()->name() . "\n";
  return $x;
}

<<__Memoize(#MakeICInaccessible)>>
function memoNoArgExn() :mixed{
  throw new Exception("oh no exception");
}

<<__Memoize(#MakeICInaccessible)>>
function memoExn($x) :mixed{
  throw new Exception("oh no exception");
}

function f() :mixed{
  $v = vec[
    tuple('memoNoArgOK', () ==> memoNoArgOK()),
    tuple('memoOK', () ==> memoOK(1)),
    tuple('memoNoArgFail', () ==> memoNoArgFail()),
    tuple('memoFail', () ==> memoFail(1)),
    tuple('memoNoArgExn', () ==> memoNoArgExn()),
    tuple('memoExn', () ==> memoExn(1)),
  ];
  foreach ($v as list($name, $f)) {
    echo "  Before: " . ClassContext::getContext()->name() . "\n";
    try {
      $f();
      echo $name . " passed\n";
    } catch (Exception $e) {
      echo $name . " failed with: " . $e->getMessage() . "\n";
    }
    echo "  After: " . ClassContext::getContext()->name() . "\n";
    echo "------------------------\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new C, f<>);
}
