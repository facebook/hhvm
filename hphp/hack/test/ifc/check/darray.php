<?hh

class C {
  <<__Policied("PUBLIC")>>
  public int $public = 0;
  <<__Policied("PRIVATE")>>
  private int $private = 0;

  public function __construct(
    <<__Policied("PUBLIC")>>
    public darray<int,int> $dPublic,
    <<__Policied("PRIVATE")>>
    public darray<int,int> $dPrivate,
  ) {}

  <<__InferFlows>>
  public function subtypeKeyKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->dPublic = darray[$this->private => 42];
  }

  <<__InferFlows>>
  public function subtypeValKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->dPublic = darray[42 => $this->private];
  }

  <<__InferFlows>>
  public function subtypeOK(): void {
    $this->dPublic = darray[42 => 42];
  }

  <<__InferFlows>>
  public function accessKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->public = $this->dPrivate[0];
  }

  <<__InferFlows>>
  public function accessOK(): void {
    $this->public = $this->dPublic[0];
  }

  <<__InferFlows>>
  public function overrideValKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->dPublic[0] = $this->private;
  }

  <<__InferFlows>>
  public function overrideKeyKO(): void {
    // PRIVATE leaks to PUBLIC
    $this->dPublic[$this->private] = 0;
  }

  <<__InferFlows>>
  public function overrideOK(): void {
    $this->dPublic[0] = 42;
  }
}
