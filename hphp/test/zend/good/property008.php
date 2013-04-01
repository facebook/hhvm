<?php
error_reporting(E_ALL | E_STRICT);

class BaseWithPropA {
  private $hello = 0;
}

// This is how privates are handled in normal inheritance
class SubclassClassicInheritance extends BaseWithPropA {
  private $hello = 0;
}

// And here, we need to make sure, that the traits behave the same

trait AHelloProperty {
  private $hello = 0;
}

class BaseWithTPropB {
    use AHelloProperty;
}

class SubclassA extends BaseWithPropA {
    use AHelloProperty;
}

class SubclassB extends BaseWithTPropB {
    use AHelloProperty;
}

$classic = new SubclassClassicInheritance;
var_dump($classic);

$a = new SubclassA;
var_dump($a);

$b = new SubclassB;
var_dump($b);

?>