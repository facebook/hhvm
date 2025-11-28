<?hh

trait T1 {
  <<__LateInit>>
  protected int $member1;
  <<__LateInit>>
  protected static int $member2;
  public function __construct(int $member1) {
    $this->member1 = $member1;
  }
}

class C1 {
  use T1;
  public function __construct() {}
}


trait T2 {
  <<__LateInit>>
  protected int $member1;
  <<__LateInit>>
  protected static int $member2;
}

class C2 {
  use T2;
  public function __construct() {}
}
