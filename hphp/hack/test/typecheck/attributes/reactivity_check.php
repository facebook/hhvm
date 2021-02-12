<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class FAttr implements HH\FunctionAttribute, HH\MethodAttribute {
  public function __construct(public string $s) {}
}

<<__Pure, FAttr('RXException')>>
async function rx(): Awaitable<void> {}

class C {
  <<__Pure, FAttr('RXException')>>
  public async function rx(): Awaitable<void> {}
}

<<__Pure, FAttr(1)>>
async function ry(): Awaitable<void> {}
