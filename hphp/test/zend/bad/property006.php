<?php
error_reporting(E_ALL | E_STRICT);

class Base {
  private $hello;    
}

trait THello1 {
  private $hello;
}

// Now we use the trait, which happens to introduce another private variable
// but they are distinct, and not related to each other, so no warning.
echo "PRE-CLASS-GUARD\n";
class SameNameInSubClassNoNotice extends Base {
    use THello1;
}
echo "POST-CLASS-GUARD\n";

// now the same with a class that defines the property itself,
// that should give the expected strict warning.

class Notice extends Base {
    use THello1;
    private $hello;
}
echo "POST-CLASS-GUARD2\n";
?>