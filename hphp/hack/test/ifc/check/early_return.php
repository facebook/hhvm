<?hh // strict

class PublicData {
  public function __construct(
    <<__Policied("PUBLIC")>>
    public string $data,
  ) {}
}

class PoliciedData {
  public function __construct(
    <<__Policied("SECRET")>>
    public bool $data,
  ) {}
}

function earlyReturn(PoliciedData $x, PublicData $y): void {
  if ($x->data) {
    return;
  }
  $y->data = "String";
}
