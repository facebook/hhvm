<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return null; }

function f(nonnull $x): void {
  hh_show($x);
}
