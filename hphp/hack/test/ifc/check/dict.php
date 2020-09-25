<?hh // strict

class Basic {
  <<Policied("S")>>
  public string $string = "string";
  <<Policied("B")>>
  public bool $bool = false;
  <<Policied("D")>>
  public dict<string,bool> $dict = dict[];

  public function set(): void {
    // B flows into D through dictionary value
    $this->dict["ludwig"] = $this->bool;
  }

  public function collection(): void {
    // B flows into D through dictionary value
    // S flows into D through dictionary key
    $this->dict = dict[$this->string => $this->bool];
  }

  public function mutationKeyLeak(): void {
    $this->dict[$this->string] = false; // S leaks to D through the key
  }

  public function accessKeyLeak(dict<string, bool> $dict): void {
    $this->bool = $dict[$this->string]; // S leaks to B
  }
}

class COW {
  public function __construct(
    <<Policied("X")>>
    public bool $x,
    <<Policied("Y")>>
    public bool $y,
    <<Policied("DICT")>>
    public dict<string,bool> $dict,
  ) {}

  public function copyOnWrite(dict<string, bool> $dict): void {
    $dict["x"] = $this->x;
    // X flows into DICT through dictionary value
    $this->dict = $dict;
    $dict["y"] = $this->y;
    // Y doesn't flow into DICT because dict has copy-on-write semantics
  }
}
