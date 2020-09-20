<?hh // strict
abstract class A {
  protected darray<string, varray<int>> $events = darray[];

  <<__RxShallow>>
  public static function eventMutable(<<__Mutable>>A $a): void {
    $a->events['$event'][0] = 1;
  }
}
