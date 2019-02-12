<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test() {
  var_dump(DateTimeImmutable::createFromFormat('Y-m-d', "blahblah"));
}
test();
