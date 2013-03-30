<?php
error_reporting(E_ALL | E_STRICT);

class Base {
  private $hello;    
}

trait THello1 {
  private $hello;
}

echo "PRE-CLASS-GUARD\n";
class Notice extends Base {
    use THello1;
    private $hello;
}
echo "POST-CLASS-GUARD\n";

// now we do the test for a fatal error

class TraitsTest {
	use THello1;
    public $hello;
}

echo "POST-CLASS-GUARD2\n";

$t = new TraitsTest;
$t->hello = "foo";
?>