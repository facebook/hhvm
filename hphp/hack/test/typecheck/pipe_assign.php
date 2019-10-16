<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function foo(): void {
  $$ = 1;
  $$ = 2; // error here
}
