<?hh

// FILE: simple.php

function simple() {
  echo "1\n";
}

// FILE: PACKAGES.toml VERSION: 3

[packages]

[packages.prod]
include_paths=["//"]

[deployments]

[deployments.prod]
packages=["prod"]

// FILE: simple.php VERSION: 6

function simple() {
  echo "2\n";
}

// FILE: main.php

<<__EntryPoint>>
function main() :mixed{
  simple();

  echo "kEvicted:\n";
  var_dump(HH\Lib\Vec\map(HH\get_compiled_units(-1), $v ==> basename($v)));
  echo "kEvicted, kCompiled:\n";
  var_dump(HH\Lib\Vec\map(HH\get_compiled_units(0), $v ==> basename($v)));
  echo "kEvicted, kCompiled, kHitDisk:\n";
  var_dump(HH\Lib\Vec\map(HH\get_compiled_units(1), $v ==> basename($v)));
  echo "kEvicted, kCompiled, kHitDisk, kWaited:\n";
  var_dump(HH\Lib\Vec\map(HH\get_compiled_units(2), $v ==> basename($v)));
}

