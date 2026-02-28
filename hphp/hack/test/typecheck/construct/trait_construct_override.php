<?hh

trait MyTrait {
  protected string $member;
  public function __construct(string $member) {
    $this->member = $member;
  }

  final public function say(): void {
    invariant($this->member !== null, 'bad');
  }
}

class MyChild {
  use MyTrait;

  public function __construct() {}
}
