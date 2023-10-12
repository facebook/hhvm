<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(): void {
  $v = Vector {};
  $x = $v[0];
  $result = tuple($x->id, $x->name);
}
