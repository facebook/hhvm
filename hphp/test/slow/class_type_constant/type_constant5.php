<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  abstract const type;
}

class C implements I {
  const type = 1;
}

var_dump(C::type - 1);
var_dump(C::type + 1);
