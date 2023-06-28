<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const FOO = (true ? 1 : 2);
}


<<__EntryPoint>>
function main_ternary() :mixed{
var_dump(A::FOO);
}
