<?hh

class Basic {
  <<__Policied("S")>>
  public string $string = "string";
  <<__Policied("B")>>
  public bool $bool = false;
  <<__Policied("D")>>
  public dict<string,bool> $dict = dict[];

  <<__InferFlows>>
  public function set(): void {
    // B flows into D through dictionary value
    $this->dict["ludwig"] = $this->bool;
  }

  <<__InferFlows>>
  public function collection(): void {
    // B flows into D through dictionary value
    // S flows into D through dictionary key
    $this->dict = dict[$this->string => $this->bool];
  }

  <<__InferFlows>>
  public function mutationKeyLeak(): void {
    $this->dict[$this->string] = false; // S leaks to D through the key
  }

  <<__InferFlows>>
  public function accessKeyLeak(dict<string, bool> $dict): void {
    $this->bool = $dict[$this->string]; // S leaks to B
  }
}

class COW {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("X")>>
    public bool $x,
    <<__Policied("Y")>>
    public bool $y,
    <<__Policied("DICT")>>
    public dict<string,bool> $dict,
  ) {}

  <<__InferFlows>>
  public function copyOnWrite(dict<string, bool> $dict): void {
    $dict["x"] = $this->x;
    // X flows into DICT through dictionary value
    $this->dict = $dict;
    $dict["y"] = $this->y;
    // Y doesn't flow into DICT because dict has copy-on-write semantics
  }
}
