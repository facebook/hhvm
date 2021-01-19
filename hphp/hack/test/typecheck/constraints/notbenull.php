<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {
  public function foo():void;
}

final class ExpectObj<T> {

  final public function __construct(
    protected T $obj,
  ): void {}

  final public function toNotBeNull<Tu>(
    string $msg = '',
  ): Tu where T = ?Tu {
    if ($this->obj is null) {
      throw new Exception("E");
    }
    return $this->obj;
  }
}

function expect<T>(T $obj): ExpectObj<T> {
  return new ExpectObj($obj);
}

async function makeIt():Awaitable<?I> {
  return null;
}

async function foo():Awaitable<void> {
  $x = await makeIt();
  $y = expect($x)->toNotBeNull();
  $y->foo();
}
