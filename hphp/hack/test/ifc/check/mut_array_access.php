<?hh

class D {}

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PRIVATE")>>
    public int $privateInt,
    <<__Policied("PUBLIC")>>
    public int $publicInt,

    <<__Policied("PRIVATE")>>
    public D $privateD,
    <<__Policied("PUBLIC")>>
    public D $publicD,
  ) {}
}

<<__InferFlows>>
function leak_via_vector_prim_value(C $c): void {
  $vector = Vector {$c->privateInt};
  // PRIVATE leaks to PUBLIC
  $c->publicInt = $vector[0];
}

<<__InferFlows>>
function leak_via_vector_object_value(C $c): void {
  $vector = Vector {$c->privateD};
  // PRIVATE leaks to PUBLIC
  $c->publicD = $vector[0];
}

<<__InferFlows>>
function leak_via_vector_index(C $c, Vector<int> $vector): void {
  // PRIVATE leaks to PUBLIC
  $c->publicInt = $vector[$c->privateInt];
}

<<__InferFlows>>
function leak_via_map_value(C $c, Map<int,int> $vector): void {
  $vector = Map {0 => $c->privateInt};
  // PRIVATE leaks to PUBLIC
  $c->publicInt = $vector[0];
}

<<__InferFlows>>
function leak_via_map_key(C $c, Map<int,int> $vector): void {
  $vector = Map {$c->privateInt => 0};
  // PRIVATE leaks to PUBLIC
  $c->publicInt = $vector[0];
}

<<__InferFlows>>
function ok(C $c): void {
  $vector = Vector {$c->publicInt};
  $c->publicInt = $vector[0];
}
