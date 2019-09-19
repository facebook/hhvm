<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function test(): void {
  $o = new stdClass();
  $o->{''} = true;
  var_dump($o);
}
