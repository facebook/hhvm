<?hh

abstract final class StaticUtils { // error: final and concrete
  <<__NeedsConcrete>>
  public static function m(): void {}
}

<<__ConsistentConstruct>>
interface ICC {}

abstract final class WithCC implements ICC { // ok: not concrete
  <<__NeedsConcrete>>
  public static function m(): void {}
}
