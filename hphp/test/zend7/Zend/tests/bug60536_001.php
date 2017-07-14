<?php
trait T { private $x = 0; }
class X {
	use T;
}
class Y extends X {
	  use T;
	  function __construct() {
	      return ++$this->x;
      }
}
class Z extends Y {
	  function __construct() {
		  return ++$this->x;
      }
}
$a = new Z();
$a->__construct();
echo "DONE";
?>
