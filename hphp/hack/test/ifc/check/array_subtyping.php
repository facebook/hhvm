<?hh

class Simple {
  <<Policied("INDEX")>> public int $ix = 42;
  <<Policied("VALUE")>> public string $value = "hello";

  public vec<string> $vec = vec[];
  <<Policied("KEYED_TRAVERSABLE")>>
  public KeyedTraversable<int,string> $keyedTraversable = vec[];
  <<Policied("KEYED_CONTAINER")>>
  public KeyedContainer<int,string> $keyedContainer = vec[];

  public function vecToKT(): void {
    $this->vec[$this->ix] = $this->value;
    // INDEX flows to KEYED_TRAVERSABLE
    // VALUE flows to KEYED_TRAVERSABLE
    $this->keyedTraversable = $this->vec;
  }

  public function vecToKC(): void {
    $this->vec[$this->ix] = $this->value;
    // INDEX flows to KEYED_TRAVERSABLE
    // VALUE flows to KEYED_TRAVERSABLE
    $this->keyedContainer = $this->vec;
  }
}
