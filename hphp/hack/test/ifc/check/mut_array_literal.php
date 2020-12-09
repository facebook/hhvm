<?hh

class D {}

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("CONTAINER")>>
    public Container<int> $container,
    <<__Policied("CONTAINER_COMPLEX")>>
    public Container<D> $containerComplex,
    <<__Policied("KEYED_CONTAINER")>>
    public KeyedContainer<string, int> $keyedContainer,

    <<__Policied("PUBLIC")>>
    public int $publicInt,
    <<__Policied("PUBLIC")>>
    public string $publicString,

    <<__Policied("VALUE_PRIM1")>>
    public int $valuePrim1,
    <<__Policied("VALUE_PRIM2")>>
    public int $valuePrim2,
    <<__Policied("VALUE_COMPLEX")>>
    public D $valueComplex,
    <<__Policied("KEY")>>
    public string $key,
  ) {}
}

<<__InferFlows>>
function ok(C $c): void {
  $c->container = Vector {1, 2, 3};
  $c->container = Set {1, 2, 3};
  $c->keyedContainer = Map {'a' => 1, 'b' => 2, 'c' => 3};

  $c->container = Vector {$c->publicInt, $c->publicInt};
  $c->container = Set {$c->publicInt};
  $c->keyedContainer =
    Map {$c->publicString => $c->publicInt, $c->publicString => $c->publicInt};
}

<<__InferFlows>>
function leak_through_simple_val(C $c): void {
  $c->container = Vector {1, $c->valuePrim1, 2, $c->valuePrim2};
}

<<__InferFlows>>
function leak_through_simple_key_val(C $c): void {
  $c->container = Map {$c->key => $c->valuePrim1, 'hi' => $c->valuePrim2};
}

<<__InferFlows>>
function leak_through_complex_val(C $c): void {
  $c->containerComplex = Vector {$c->valueComplex};
}
