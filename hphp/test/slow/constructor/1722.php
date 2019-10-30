<?hh

if (isset($g)) {
  include '1722-1.inc';
}
else {
  include '1722-2.inc';
}
class Y extends X {
  function __construct($a, $b) {
    var_dump(__METHOD__);
    parent::__construct($a,$b);
  }
}
$y = new Y(1,2);
