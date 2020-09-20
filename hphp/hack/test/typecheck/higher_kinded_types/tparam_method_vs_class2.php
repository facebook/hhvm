<?hh // strict

class Test<TA<TB>> {

  // error: TB is not bound anymore from class tparam def, but we still forbid
  // reusing the name
  public function test1<TB>() : void {}

  // same in nested positions
  public function test1<TC<TB>>() : void {}

}
