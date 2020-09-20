<?hh // strict

class PublicData {
  public function __construct(
    <<Policied("PUBLIC")>>
    public string $data,
  ) {}
}

class PoliciedData {
  public function __construct(
    <<Policied("SECRET")>>
    public bool $data,
  ) {}
}

function earlyReturn(PoliciedData $x, PublicData $y): void {
  if ($x->data) {
    return;
  }
  $y->data = "String";
}
