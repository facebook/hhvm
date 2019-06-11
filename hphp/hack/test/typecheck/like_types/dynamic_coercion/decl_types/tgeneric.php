<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

function erase<Terase>(): Terase {
  return dyn(); // error;
}

function reify<reify Treify>(): Treify {
  return dyn(); // error
}

function enforce<<<__Enforceable>> reify Tenforce>(): Tenforce {
  return dyn(); // ok
}
