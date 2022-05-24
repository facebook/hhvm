<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
interface ITop<T as supportdyn<mixed>> {}

<<__SupportDynamicType>>
interface INo<T as supportdyn<mixed>> extends ITop<T> {}

<<__SupportDynamicType>>
interface IYes<T as supportdyn<mixed>> extends ITop<T> {
  public function f(T $e): void;
}

final class Local {
  public static function fail<T as supportdyn<mixed>>(
    ~ITop<T> $processor,
    T $e,
  ): void {
    if ($processor is INo<_>) {
      // nothing
    } else if ($processor is IYes<_>) {
      $processor->f($e);
    }
  }
}
