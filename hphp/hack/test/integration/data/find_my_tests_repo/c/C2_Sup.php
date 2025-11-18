<?hh

// Note that the C2_* classes have an explicit __construct

class C2_Sup {

  public function __construct() {
    $this->i = 123;
  }

  private int $i;

}
