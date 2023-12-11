<?hh
namespace A {
  // SORT_REGULAR is a global constant
  const SORT_NUMERIC = 42;

  function c($k = \SORT_REGULAR) :mixed{ \var_dump($k); }
  function d($k = \SORT_NUMERIC) :mixed{ \var_dump($k); }

  // A\B\SORT_NUMERIC
  function e($k = B\SORT_NUMERIC) :mixed{ \var_dump($k); }

  function f($k = \B\SORT_NUMERIC) :mixed{ \var_dump($k); }
}

namespace B {
  const SORT_NUMERIC = 'B val';
}
namespace A\B {
  const SORT_NUMERIC = 'A\B val';
}

namespace {
<<__EntryPoint>> function main(): void {
  foreach (vec['c', 'd', 'e', 'f'] as $func) {
    echo "A\\$func reflection:\n";
    $rc = new ReflectionFunction("A\\$func");
    \var_dump($rc->getParameters()[0]->getDefaultValue());
    \var_dump($rc->getParameters()[0]->getDefaultValueText());
    \var_dump($rc->getParameters()[0]->getDefaultValueConstantName());
    echo "\n";
    echo "A\\$func call:\n";
    call_user_func("A\\$func");
    echo "\n";
  }
}
}
