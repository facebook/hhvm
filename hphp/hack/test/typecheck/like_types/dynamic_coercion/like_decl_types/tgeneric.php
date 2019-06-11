<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function erase<Terase>(~Terase $i): Terase {
  return $i; // error;
}

function reify<reify Treify>(~Treify $i): Treify {
  return $i; // error
}

function enforce<<<__Enforceable>> reify Tenforce>(~Tenforce $i): Tenforce {
  return $i; // ok
}
