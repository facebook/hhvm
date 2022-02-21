<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function repro(?(function()[_]: void) $comp = null)[ctx $comp]: void
{}

function toplevel():void {
  $r = repro<>;
  $r();
}
