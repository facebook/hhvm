<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class :foo extends XHPTest implements XHPChild {
  attribute enum {1, "stuff"} bar;
}

function bad_attr_value(): void {
  $x = <foo bar={3} />;
}
