<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class WithTyConst {
  abstract const type T;
}

class Box<T> {
  public function __construct(private T $thing) {}
  public function getThing(): T {
    return $this->thing;
  }
}

class Processor {
  public static function extractThing<TKey as arraykey>(
    Box<TKey> $box,
  ): vec<TKey> {
    return vec[$box->getThing()];
  }

  public static function extractThingRedundant<
    T as Box<TKey>,
    TKey as arraykey,
  >(T $box): vec<TKey> {
    return vec[$box->getThing()];
  }

  public static function returnBoxOpt<T as Box<TKey>, TKey as arraykey>(
    T $box,
  ): ?T {
    return null;
  }

  public static function doSomething<
    T1 as WithTyConst with { type T = T2 },
    T2 as arraykey,
  >(T1 $x): ?T2 {
    return null;
  }

  public static function doSomethingElse<
    T1 as WithTyConst with { type T = T2 },
    T2 as arraykey,
  >(T1 $x): int {
    return 0;
  }
}

function test_static_method_ref(): void {
  $f = Processor::extractThing<>;
  hh_expect<
    HH\FunctionRef<(readonly function<TKey as arraykey>(Box<TKey>): vec<TKey>)>,
  >($f);

  $g = Processor::extractThingRedundant<>;
  hh_expect<
    HH\FunctionRef<(readonly function<TKey as arraykey>(Box<TKey>): vec<TKey>)>,
  >($g);

  $h = Processor::returnBoxOpt<>;
  hh_expect<
    HH\FunctionRef<(readonly function<T as Box<TKey>, TKey as arraykey>(
      T,
    ): ?T)>,
  >($h);

  $i = Processor::doSomething<>;
  hh_expect<
    HH\FunctionRef<(readonly function<T2 as arraykey>(
      WithTyConst with { type T = T2 },
    ): ?T2)>,
  >($i);

  $j = Processor::doSomethingElse<>;
  hh_expect<
    HH\FunctionRef<(readonly function<T2 as arraykey>(
      WithTyConst with { type T = T2 },
    ): int)>,
  >($j);
}
