<?hh

class PublicData {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PUBLIC")>>
    public string $data,
  ) {}
}

class PoliciedData {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("SECRET")>>
    public bool $data,
  ) {}
}

<<__InferFlows>>
function earlyReturn(PoliciedData $x, PublicData $y): void {
  if ($x->data) {
    return;
  }
  $y->data = "String";
}
