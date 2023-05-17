<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
interface I {
  public function bar():~Awaitable<dynamic>;
}

<<__SupportDynamicType>>
class C {
  public async function foo():Awaitable<dynamic> {
    return "A" upcast dynamic;
  }
}
<<__SupportDynamicType>>
class D extends C implements I {
  public async function foo():Awaitable<int> {
    return 5;
  }
  public async function bar():Awaitable<int> {
    return 5;
  }
}
