<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract final class C {}

function foo(): void {
  $_ = (): ?C ==> {
    return null;
  };
}
