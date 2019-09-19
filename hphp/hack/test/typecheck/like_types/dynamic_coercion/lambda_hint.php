<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f((function (int): void) $f, dynamic $d): void {
  $f($d); // error
}
