<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

function tnull(): null {
  return dyn(); // ok
}

function tvoid(): void {
  return dyn(); // cannot return
}

function tint(): int {
  return dyn(); // ok
}

function tbool(): bool {
  return dyn(); // ok
}

function tstring(): string {
  return dyn(); // ok
}

function tresource(): resource {
  return dyn(); // ok
}

function tnum(): num {
  return dyn(); // ok
}

function tarraykey(): arraykey {
  return dyn(); // ok
}

function tnoreturn(): noreturn {
  return dyn(); // cannot return
}
