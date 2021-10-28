<?hh

trait MyTrait {
  private int $limit;
}

trait OtherTrait {
  use MyTrait;

  public function __construct() {
    $this->limit = 2;
  }
}

class MyClassUsesMyTrait {
  use OtherTrait;
  public function __construct() {}

  // Fatal: Expected int, got null.
  public function getLimit(): int {
    return $this->limit;
  }
}

class OtherClassThatUsesMyTrait {
  use OtherTrait;
  public function __construct() {}
}
