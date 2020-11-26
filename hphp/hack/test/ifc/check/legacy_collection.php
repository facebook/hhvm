<?hh

class C {
  <<__Policied("KEY")>>
  public int $key = 0;
  <<__Policied("VALUE")>>
  public int $value = 0;
  <<__Policied("PUBLIC")>>
  public int $public = 0;

  <<__InferFlows>>
  public function leak_imm_vector(): void {
    $v = ImmVector { $this->value };
    $this->public = $v[0];
  }

  <<__InferFlows>>
  public function leak_imm_set(): void {
    $s = ImmSet { $this->value };
    foreach ($s as $v) {
      $this->public = $this->value;
    }
  }

  <<__InferFlows>>
  public function leak_imm_map(): void {
    $m = ImmMap { $this->key => $this->value };
    $this->public = $m[0];
  }
}
