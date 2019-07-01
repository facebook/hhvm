<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function erase<Terase>(Terase $x): void {
  hh_show($x);
}

function reify<reify Treify>(Treify $x): void {
  hh_show($x);
}

function enforce<<<__Enforceable>> reify Tenforce>(Tenforce $x): void {
  hh_show($x);
}
