<?hh

class C {
  <<__Policied("PUBLIC")>>
  public int $public = 0;
  <<__Policied("PRIVATE")>>
  public int $private = 0;
  <<__Policied("PRIVATE")>>
  public vec<int> $privateVec = vec[];
  <<__Policied("KEY")>>
  public int $key = 0;
  <<__Policied("VALUE")>>
  public int $value = 0;

  <<__InferFlows>>
  public function traverse(Traversable<int> $t): void {
    foreach ($t as $value) {
      $this->value = $value;
    }
  }

  <<__InferFlows>>
  public function traverse_no_assign(Traversable<int> $t): void {
    foreach ($t as $x) {
      $this->public = 0;
    }
  }

  <<__InferFlows>>
  public function vec(vec<int> $t): void {
    foreach ($t as $value) {
      $this->value = $value;
    }
  }

  <<__InferFlows>>
  public function keyedTraverse(KeyedTraversable<int,int> $t): void {
    foreach ($t as $key => $value) {
      $this->key = $key;
      $this->value = $value;
    }
  }

  <<__InferFlows>>
  public function dict(dict<int,int> $t): void {
    foreach ($t as $key => $value) {
      $this->key = $key;
      $this->value = $value;
    }
  }

  public function ok(): void {
    $this->traverse(vec[$this->value]);
    $this->vec(vec[$this->value]);

    $this->traverse(vec[$this->public]);
    $this->keyedTraverse(dict[$this->public => $this->public]);
    $this->dict(dict[$this->public => $this->public]);

    $this->traverse(vec[0]);
    $this->keyedTraverse(dict[0 => 0]);
    $this->dict(dict[0 => 0]);
  }

  <<__InferFlows>>
  public function leaks_due_to_key_value_conflation(): void {
    // KEY and VALUE are both governed by the lump policy of the Traversable in
    // keyedTraverse. Thus, unlike using foreach on a CoW array, we lose
    // specificity.
    $this->keyedTraverse(dict[$this->key => $this->value]);
  }

  <<__InferFlows>>
  public function leak_via_value1(): void {
    // PRIVATE leaks to VALUE
    $this->traverse(vec[$this->private]);
  }

  <<__InferFlows>>
  public function leak_via_value2(): void {
    // PRIVATE leaks to KEY
    // PRIVATE leaks to VALUE
    $this->keyedTraverse(dict[0 => $this->private]);
  }

  <<__InferFlows>>
  public function leak_via_key1(): void {
    // PRIVATE leaks to KEY
    // PRIVATE leaks to VALUE
    $this->keyedTraverse(dict[$this->private => 0]);
  }

  <<__InferFlows>>
  public function leak_via_key2(): void {
    $vec = vec[];
    $vec[$this->private] = 42;
    // PRIVATE leaks to KEY
    // PRIVATE leaks to VALUE
    $this->keyedTraverse($vec);
  }

  <<__InferFlows>>
  public function leak_implicitly(): void {
    // PRIVATE leaks to PUBLIC due to implicit flow from length (governed by
    // self policy)
    $this->traverse_no_assign($this->privateVec);
  }
}
