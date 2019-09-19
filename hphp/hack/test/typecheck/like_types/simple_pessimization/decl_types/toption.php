<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function enforceable(?int $x): void {
  hh_show($x);
}

function unenforceable(?(int, int) $x): void {
  hh_show($x);
}
