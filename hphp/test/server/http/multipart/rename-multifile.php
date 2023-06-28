<?hh

// FILE: foo.php

class Foo {
  function go() :mixed{ echo __CLASS__." in ".basename(__FILE__)."\n"; }
}

// FILE: foo.php VERSION: 1

class Bar {
  function go() :mixed{ echo __CLASS__." in ".basename(__FILE__)."\n"; }
}

// FILE: bar.php

class Bar {
  function go() :mixed{ echo __CLASS__." in ".basename(__FILE__)."\n"; }
}

// FILE: bar.php VERSION: 1

class Foo {
  function go() :mixed{ echo __CLASS__." in ".basename(__FILE__)."\n"; }
}

// FILE: alias1.php

newtype TAlias = Foo;

// FILE: alias1.php VERSION: 1

// empty

// FILE: alias2.php

// empty

// FILE: alias2.php VERSION: 1

newtype TAlias = Bar;

// FILE: main.php

function test(TAlias $x) :mixed{ echo "test(".get_class($x).") = ok\n"; }
function go(classname<mixed> $n) :mixed{
  if (!class_exists($n)) { echo "test($n) = no\n"; return; }
  try { test(new $n); } catch (Exception $e) { echo "test($n) = ex\n"; }
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(() ==> { throw new Exception; }, E_RECOVERABLE_ERROR);

  if (class_exists(Foo::class)) (new Foo)->go();
  else                          echo "No Foo\n";
  if (class_exists(Bar::class)) (new Bar)->go();
  else                          echo "No Bar\n";

  go(Foo::class);
  go(Bar::class);
}
