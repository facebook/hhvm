<?hh

trait T1 {
  <<__LateInit>>
  protected string $member1;
  <<__LateInit>>
  protected static string $member2;
  public function __construct(string $member1) {
    $this->member1 = $member1;
  }
}

class C1 {
  use T1;
  public function __construct() {}
}


trait T2 {
  <<__LateInit>>
  protected string $member1;
  <<__LateInit>>
  protected static string $member2;
}

class C2 {
  use T2;
  public function __construct() {}
}
