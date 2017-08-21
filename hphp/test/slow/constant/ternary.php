<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const FOO = (true ? 1 : 2);
}

var_dump(A::FOO);
