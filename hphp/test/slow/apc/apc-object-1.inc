<?hh

class X {
  private $foo;
  function __construct() { $this->foo='bar'; }
  function show() { var_dump($this->foo); }
};

$x = 1;
