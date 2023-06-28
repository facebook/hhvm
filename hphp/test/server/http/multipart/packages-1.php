<?hh

// FILE: PACKAGES.toml
[packages]

[packages.foo]
uses = ["a.*"]

// FILE: PACKAGES.toml VERSION: 1
[packages]

[packages.foo]
uses = ["a.*"]

[packages.bar]
uses = ["b.*"]
includes = ["foo"]

// FILE: a.php

function foo() :mixed{ return 1; }

// FILE: a.php VERSION: 1

function foo() :mixed{ return 2; }

// FILE: main.php

<<__EntryPoint>>
function main(): void {
  var_dump(foo());
  var_dump(HH\Lib\Vec\keys(HH\get_all_packages()));
}
