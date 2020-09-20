<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(dynamic $d): void {
  // function chr(int $ascii): string;
  // $ascii is now ~int
  chr($d);

  // function is_vec(mixed $arg): bool;
  // bool is now ~bool
  hh_show(HH\is_vec($d));
}
