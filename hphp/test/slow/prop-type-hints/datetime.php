<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>> function test(): void {
  var_dump(DateTimeImmutable::createFromFormat('Y-m-d', "blahblah"));
}
