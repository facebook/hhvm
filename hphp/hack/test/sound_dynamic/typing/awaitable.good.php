<?hh

<<__SupportDynamicType>>
class Box {
  public function __construct(public int $data) {}
}

<<__SupportDynamicType>>
class D {
  public async function foo() : Awaitable<void> {
  }

  public async function box(): Awaitable<Box> {
    return new Box(42);
  }
}
