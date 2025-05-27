<?hh

<<file: __EnableUnstableFeatures('type_splat', 'open_tuples')>>

class Other<T as (mixed...)> { }

abstract class Good {
  abstract const type TInputs as (arraykey...);

  public function test1((function(...this::TInputs): void) $x):void { }
  public function test2(...this::TInputs $args):void { }
  public function test3(Other<this::TInputs> $a):void { }
}

abstract class Bad {
  abstract const type TInputs;

  public function test1((function(...this::TInputs): void) $x):void { }
  public function test2(...this::TInputs $args):void { }
  public function test3(Other<this::TInputs> $a):void { }
}
