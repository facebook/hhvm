<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class Cfinal {}

function f<<<__Newable>> reify T as Cfinal>(): void {
  new T();
}

function g(): void {
  f<Cfinal>();
}
