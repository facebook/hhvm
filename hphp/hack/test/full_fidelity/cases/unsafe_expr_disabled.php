<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.


function foo(): int {
  return /* UNSAFE_EXPR */ "sheep";
}
