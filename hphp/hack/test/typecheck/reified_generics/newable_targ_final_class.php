<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class Cfinal {}

function f<<<__Newable>> T as Cfinal>(): void {}

function g(): void {
  f<Cfinal>();
}
