<?hh

class Y extends X {
  function __construct($a, $b) {
    var_dump(__METHOD__);
    parent::__construct($a,$b);
  }
}
