<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class FAttr implements HH\FunctionAttribute, HH\MethodAttribute {
  public function __construct(public string $s) {}
}

<<__Rx, FAttr('RXException')>>
async function rx(): Awaitable<void> {}

class C {
  <<__Rx, FAttr('RXException')>>
  public async function rx(): Awaitable<void> {}
}

<<__Rx, FAttr(1)>>
async function ry(): Awaitable<void> {}
