<?hh

class C {
  private function __construct($a) {}
  public function __construct($a = null) {}
  public function __construct($a) {}
  private function __construct($a = null) {}
  <<attr>> public function __construct ($a){}
  <<attr>> public function __construct ($a) {}
  public function __construct (public $a = null) {}
}
