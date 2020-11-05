<?hh

class C {
  <<__Policied("PUBLIC")>>
  public int $public = 0;
  <<__Policied("PRIVATE")>>
  private int $private = 0;

  public function __construct(
    <<__Policied("PUBLIC")>>
    public varray<int> $vPublic,
    <<__Policied("PRIVATE")>>
    public varray<int> $vPrivate,
  ) {}

  <<__InferFlows>>
  public function subtypeKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->vPublic = varray[$this->private];
  }

  <<__InferFlows>>
  public function subtypeOK(): void {
    $this->vPublic = varray[42];
  }

  <<__InferFlows>>
  public function accessKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->public = $this->vPrivate[0];
  }

  <<__InferFlows>>
  public function accessOK(): void {
    $this->public = $this->vPublic[0];
  }

  <<__InferFlows>>
  public function addKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->vPublic[] = $this->private;
  }

  <<__InferFlows>>
  public function addOK(): void {
    $this->vPublic[] = 42;
  }

  <<__InferFlows>>
  public function overrideValKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->vPublic[0] = $this->private;
  }

  <<__InferFlows>>
  public function overrideKeyKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->vPublic[$this->private] = 0;
  }

  <<__InferFlows>>
  public function overrideOK(): void {
    $this->vPublic[0] = 42;
  }
}
