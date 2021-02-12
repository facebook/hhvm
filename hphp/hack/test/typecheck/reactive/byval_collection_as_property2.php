<?hh // strict
class A {
  protected darray<string, int> $events = darray[];


  public function eventMutable(?string $event)[]: void {
    if ($event !== null) {
      // ERROR
      $this->events[$event] = 1;
    }
  }
}
