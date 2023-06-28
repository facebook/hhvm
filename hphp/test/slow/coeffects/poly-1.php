<?hh

interface I {
  abstract const ctx C;
}

class Foo1 implements I {
  const ctx C = [rx];
}

class Foo2 implements I {
  const ctx C = [];
}

class Foo3 implements I {
  const ctx C = [defaults];
}

function f1(I $a = null)[$a::C] :mixed{
  echo "in f1\n";
}

function f2(I $a = null)[write_props, $a::C] :mixed{
  echo "in f2\n";
}

function pure($f, $a)[] :mixed{ $f($a); }
function rx($f, $a)[rx] :mixed{ $f($a); }
function defaults($f, $a) :mixed{ $f($a); }

<<__EntryPoint>>
function main() :mixed{
  $callers = vec['pure', 'rx', 'defaults'];
  $callees = vec[
    tuple('f1', new Foo1()),
    tuple('f1', new Foo2()),
    tuple('f1', new Foo3()),
    tuple('f2', new Foo1()),
    tuple('f2', new Foo2()),
    tuple('f2', new Foo3()),
  ];
  foreach ($callers as $caller) {
    echo "=== $caller ===\n";
    foreach ($callees as $callee_pair) {
      list($callee, $arg) = $callee_pair;
      $caller($callee, $arg);
      echo "$callee: ok\n";
    }
  }
}
