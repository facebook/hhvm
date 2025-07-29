<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
final class C {
  public async function genData(
  ): Awaitable<~this::TC> {
    throw new Exception("A");
  }
  const type TC = shape('a' => vec<int>);

  private ~?this::TC $data = null;

  public async function genSomething(vec<int> $v): Awaitable<~vec<int>> {
    $this->data = await $this->genData();
    return $this->data['a'];
  }
}

<<__SupportDynamicType>>
final class D {
  public ?~vec<int> $p = null;
  public function appendSomething(vec<int> $v): void {
    $this->p ??= vec[];
    $this->p[] = 5;
  }
}
