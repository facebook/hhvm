<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class :foo extends XHPTest implements XHPChild {
  attribute enum {'big', 'small'} size;
}

function bad_attr_value(): void {
  $x = <foo size="medium" />;
}
