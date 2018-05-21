<?hh // strict
abstract final class A {
  protected darray<string, int> $events = darray[];

  <<__RxShallow, __Mutable>>
  public function eventMutable(?string $event): void {
    if ($event !== null) {
      // OK
      $this->events[$event] = 1;
    }
  }
}
