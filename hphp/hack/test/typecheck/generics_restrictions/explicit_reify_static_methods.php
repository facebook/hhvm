<?hh

class E<<<__Explicit>> T>{
  public static function factory<<<__Explicit>> TF>() : E<TF> {
    return new E<TF>();
  }
  public static function bad_factory() : E<T> {
    return new E<T>();
  }
}

class R<reify T>{
  public static function factory<reify TF>() : R<TF> {
    return new R<TF>();
  }
  public static function bad_factory() : R<T> {
    return new R<T>();
  }
}

class A{}

function test() : void {
  E::factory();
  E::factory<A>();
  E<A>::factory();
  E<A>::factory<A>();

  E::bad_factory();
  E<A>::bad_factory();

  R::factory();
  R::factory<A>();
  R<A>::factory();
  R<A>::factory<A>();

  R::bad_factory();
  R<A>::bad_factory();
}
