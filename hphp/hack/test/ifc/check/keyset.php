<?hh

class Basic {
  <<Policied("S")>>
  public string $string = "string";
  <<Policied("A")>>
  public arraykey $arraykey = "string";
  <<Policied("K")>>
  public keyset<arraykey> $keyset = keyset[];

  public function add(): void {
    // S flows into K
    $this->keyset[] = $this->string;
  }

  public function collection(): void {
    // S flows into K
    $this->keyset = keyset[42, $this->string];
  }

  public function access(): void {
    // K flows into A
    $this->arraykey = $this->keyset['key'];
  }
}

class COW {
  public function __construct(
    <<Policied("X")>>
    public string $x,
    <<Policied("Y")>>
    public int $y,
    <<Policied("KEYSET")>>
    public keyset<arraykey> $keyset,
  ) {}

  public function copyOnWrite(keyset<arraykey> $keyset): void {
    $keyset[] = $this->x;
    // X flows into KEYSET through keyset value
    $this->keyset = $keyset;
    $keyset[] = $this->y;
    // Y doesn't flow into KEYSET because keyset has copy-on-write semantics
  }
}
