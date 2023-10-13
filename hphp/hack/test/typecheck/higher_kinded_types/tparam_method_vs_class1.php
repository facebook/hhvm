<?hh


class Test<TA> {

  // bad: TA is already bound
  public function test<TB<TA>>() : void {}

}
