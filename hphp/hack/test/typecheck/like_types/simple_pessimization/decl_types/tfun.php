<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f((function (): string) $x): void {
  hh_show($x);
}
