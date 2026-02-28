<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface IFunc {
}

class MyFunc<T> implements IFunc {
}

enum class MyFuncsBase: IFunc {
}

abstract class MyMutatorBase {
  abstract const type TObj;
  abstract const type TFuncs as MyFuncsBase;

  final public function do<Trest as (mixed...)>(
    HH\EnumClass\Label<this::TFuncs, MyFunc<(this::TObj, ...Trest)>> $label,
    ... Trest $args,
  ): void {
  }
}

enum class MyIntFuncs: IFunc extends MyFuncsBase {
  MyFunc<(int)> increment = new MyFunc<(int)>();
  MyFunc<(int, int)> add = new MyFunc<(int, int)>();
  MyFunc<(int, int)> multiply = new MyFunc<(int, int)>();
}

final class MyIntMutator extends MyMutatorBase {
  const type TObj = int;
  const type TFuncs = MyIntFuncs;
}

function test_example(MyIntMutator $mutator): void {
  $mutator->do(#AUTO332
}
