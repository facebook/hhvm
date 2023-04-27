<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class :foo extends XHPTest implements XHPChild {
  attribute enum {1, 2} size;
}

function bad_attr_value(): void {
  $x = <foo size={3} />;
}
