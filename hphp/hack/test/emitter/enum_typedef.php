<?hh // strict

type SomeType = int;

enum Whatever : SomeType {
  FOO = 10;
}

type WhateverAlias = Whatever;

function test(): void {}
