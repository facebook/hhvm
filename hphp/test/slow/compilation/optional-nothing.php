<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function foo1(): ?nothing {
  return null;
}
function foo2(): ?nothing {
  throw new Exception("FOO");
}

<<__EntryPoint>>
function main() :mixed{
  try {
    var_dump(foo1());
    var_dump(foo2());
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
