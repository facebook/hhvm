<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

abstract class Tupper {}

<<__SupportDynamicType>>
abstract class G<T as supportdyn<mixed> as Tupper> {}

abstract class Inf<T as supportdyn<mixed> as Tupper> {
  private ~T $t;
  private async function f(): Awaitable<~G<T>> {}

  private async function infinite(): Awaitable<void> {
    $f = await $this->f();
    $f as G<_>;
    $this->t->noexist();
  }
}
