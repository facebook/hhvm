<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function nr(): noreturn {
  throw new Exception('nope');
}

function nr_delegation(): noreturn {
  return nr();
}

function nr_usage(): noreturn {
  nr(); // this should be ok, but depends on terminality calculation
}
