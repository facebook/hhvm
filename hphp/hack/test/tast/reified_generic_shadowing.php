<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Tcat {}

function f<Tcat>(): void {
  new Tcat(); // "Tcat"
}

function g(): void {
  new Tcat(); // "\Tcat"
}
