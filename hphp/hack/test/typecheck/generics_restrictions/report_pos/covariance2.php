<?hh

interface X<+T> {
  /* HH_FIXME[4120] */
  public function test(T $x): void;
  /* HH_FIXME[4120] */
  public function test2(T... $x): void;
}
