<?hh // strict

class R {
  public function __construct(
    public int $prop,
    public R $r,
  ) { }

  public function recursiveGet(): int {
    return $this->r->r->r->prop;
  }

  public function recursiveWrite(): void {
    $this->r->r->r->prop = 42;
  }
}
