<?hh

// Note that the C1_* classes have an explicit __construct

class C2_Mid extends C2_Sup {

  public function __construct() {
    parent::__construct();
  }

  public function origin(): void {}
}
