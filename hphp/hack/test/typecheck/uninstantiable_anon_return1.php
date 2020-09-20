<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract final class C {}

function foo(): void {
  $_ = function(): ?C {
    return null;
  };
}
