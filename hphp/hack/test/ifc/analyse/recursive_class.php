<?hh

class R {
  <<__InferFlows>>
  public function __construct(
    public int $prop,
    public R $r,
  ) { }

  <<__InferFlows>>
  public function recursiveGet(): int {
    return $this->r->r->r->prop;
  }

  <<__InferFlows>>
  public function recursiveWrite(): void {
    $this->r->r->r->prop = 42;
  }
}
