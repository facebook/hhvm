<?hh // strict

namespace NS10 {
  function f10(): void {
    echo "Inside namespace " . __NAMESPACE__ . "\n";
    echo "Inside function " . __FUNCTION__ . "\n";
    echo "Inside method " . __METHOD__ . "\n";
  }
}

namespace {
  function f20(): void {
    echo "Inside namespace " . __NAMESPACE__ . "\n";
    echo "Inside function " . __FUNCTION__ . "\n";
    echo "Inside method " . __METHOD__ . "\n";
  }
}

namespace NS20 {
  function f30(): void {
    echo "Inside namespace " . __NAMESPACE__ . "\n";
    echo "Inside function " . __FUNCTION__ . "\n";
    echo "Inside method " . __METHOD__ . "\n";
  }
}

namespace {
  function main_namespaces2(): void {
    \NS10\f10();
    f20();
    \NS20\f30();
  }
}

/* HH_FIXME[1002] call to main in strict*/
main_namespaces2();
