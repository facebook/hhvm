<?hh
// (c) Meta Platforms, Inc. and affiliates.

<<__SupportDynamicType>>
interface I1<TEnt as supportdyn<mixed>> {}

<<__SupportDynamicType>>
final class C1<TEnt as supportdyn<mixed>> implements I1<TEnt> {}

<<__SupportDynamicType>>
final class C2<TEnt as supportdyn<mixed>> implements I1<TEnt> {}

trait T {
  abstract const type TEnt as NoExist;

  final protected function empty(): this::TEnt::TViewerContext {}

  private function broken(~I1<this::TEnt> $i): void {
    if ($i is C1<_>) {
    } else if ($i is C2<_>) {
      $this->empty();
    }
  }
}
