<?hh

class Simple {
  <<__Policied("INDEX")>> public int $ix = 42;
  <<__Policied("VALUE")>> public string $value = "hello";

  public vec<string> $vec = vec[];
  <<__Policied("KEYED_TRAVERSABLE")>>
  public KeyedTraversable<int,string> $keyedTraversable = vec[];
  <<__Policied("KEYED_CONTAINER")>>
  public KeyedContainer<int,string> $keyedContainer = vec[];

  <<__InferFlows>>
  public function vecToKT(): void {
    $this->vec[$this->ix] = $this->value;
    // INDEX flows to KEYED_TRAVERSABLE
    // VALUE flows to KEYED_TRAVERSABLE
    $this->keyedTraversable = $this->vec;
  }

  <<__InferFlows>>
  public function vecToKC(): void {
    $this->vec[$this->ix] = $this->value;
    // INDEX flows to KEYED_TRAVERSABLE
    // VALUE flows to KEYED_TRAVERSABLE
    $this->keyedContainer = $this->vec;
  }
}
