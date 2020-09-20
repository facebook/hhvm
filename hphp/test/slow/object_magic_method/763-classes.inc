<?hh

class Y extends X {
  function foo() {
    var_dump(__METHOD__);
    parent::foo();
  }
}
class Z extends X {}
