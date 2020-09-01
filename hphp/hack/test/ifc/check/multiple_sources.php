<?hh

class C {
  public function __construct(
    <<Policied("A")>>
    public int $a,
    <<Policied("B")>>
    public int $b1,
    <<Policied("B")>>
    public int $b2,
    <<Policied("PUBLIC")>>
    public int $public,
    public int $v1,
    public int $v2,
  ) {}

  public function testDistinctPurposes(): void {
    $this->public = $this->a + $this->b1;
  }

  public function testSamePurpose(): void {
    $this->public = $this->b1 + $this->b2;
  }

  public function testIndirection1(): void {
    $this->v1 = $this->a + $this->b1;
    $this->public = $this->v1;
  }

  public function testIndirection2(): void {
    $this->v1 = $this->a;
    $this->public = $this->v1 + $this->b1;
  }

  public function testIndirection3(): void {
    $this->v1 = $this->a;
    $this->v2 = $this->b1;
    $this->public = $this->v1 + $this->v2;
  }

  public function halfIllegal(): void {
    $this->a += $this->b1;
  }
}
