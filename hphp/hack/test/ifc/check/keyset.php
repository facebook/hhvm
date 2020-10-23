<?hh

class Basic {
  <<__Policied("S")>>
  public string $string = "string";
  <<__Policied("A")>>
  public arraykey $arraykey = "string";
  <<__Policied("K")>>
  public keyset<arraykey> $keyset = keyset[];

  <<__InferFlows>>
  public function add(): void {
    // S flows into K
    $this->keyset[] = $this->string;
  }

  <<__InferFlows>>
  public function collection(): void {
    // S flows into K
    $this->keyset = keyset[42, $this->string];
  }

  <<__InferFlows>>
  public function access(): void {
    // K flows into A
    $this->arraykey = $this->keyset['key'];
  }
}

class COW {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("X")>>
    public string $x,
    <<__Policied("Y")>>
    public int $y,
    <<__Policied("KEYSET")>>
    public keyset<arraykey> $keyset,
  ) {}

  <<__InferFlows>>
  public function copyOnWrite(keyset<arraykey> $keyset): void {
    $keyset[] = $this->x;
    // X flows into KEYSET through keyset value
    $this->keyset = $keyset;
    $keyset[] = $this->y;
    // Y doesn't flow into KEYSET because keyset has copy-on-write semantics
  }
}
