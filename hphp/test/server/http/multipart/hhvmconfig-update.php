<?hh

// FILE: .hhvmconfig.hdf
Parser {
  AliasedNamespaces {
    Foo = A
  }
}

// FILE: .hhvmconfig.hdf VERSION: 1
Parser {
  AliasedNamespaces {
    Foo = B
  }
}

// FILE: a.php

namespace A;
function bar() {
  return "A\\bar";
}

// FILE: b.php

namespace B;
function bar() {
  return "B\\bar";
}

// FILE: main.php

<<__EntryPoint>>
function main(): void {
  var_dump(Foo\bar());
}
