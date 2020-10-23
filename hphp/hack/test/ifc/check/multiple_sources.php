<?hh

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("A")>>
    public int $a,
    <<__Policied("B")>>
    public int $b1,
    <<__Policied("B")>>
    public int $b2,
    <<__Policied("PUBLIC")>>
    public int $public,
    public int $v1,
    public int $v2,
  ) {}

  <<__InferFlows>>
  public function testDistinctPurposes(): void {
    $this->public = $this->a + $this->b1;
  }

  <<__InferFlows>>
  public function testSamePurpose(): void {
    $this->public = $this->b1 + $this->b2;
  }

  <<__InferFlows>>
  public function testIndirection1(): void {
    $this->v1 = $this->a + $this->b1;
    $this->public = $this->v1;
  }

  <<__InferFlows>>
  public function testIndirection2(): void {
    $this->v1 = $this->a;
    $this->public = $this->v1 + $this->b1;
  }

  <<__InferFlows>>
  public function testIndirection3(): void {
    $this->v1 = $this->a;
    $this->v2 = $this->b1;
    $this->public = $this->v1 + $this->v2;
  }

  <<__InferFlows>>
  public function halfIllegal(): void {
    $this->a += $this->b1;
  }
}
