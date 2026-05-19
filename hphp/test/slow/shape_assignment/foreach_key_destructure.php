<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

final class ShapeKeyedIterator
  implements KeyedIterator<shape('id' => int, 'tag' => string), (int, string)> {
  private int $idx = 0;

  public function current(): (int, string) {
    return vec[tuple(10, 'ten'), tuple(20, 'twenty')][$this->idx];
  }

  public function key(): shape('id' => int, 'tag' => string) {
    return vec[
      shape('id' => 1, 'tag' => 'a'),
      shape('id' => 2, 'tag' => 'b'),
    ][$this->idx];
  }

  public function next(): void {
    $this->idx++;
  }

  public function rewind(): void {
    $this->idx = 0;
  }

  public function valid(): bool {
    return $this->idx < 2;
  }
}

final class TupleKeyedIterator
  implements KeyedIterator<(int, string), shape('ok' => bool)> {
  private int $idx = 0;

  public function current(): shape('ok' => bool) {
    return vec[shape('ok' => true), shape('ok' => false)][$this->idx];
  }

  public function key(): (int, string) {
    return vec[tuple(1, 'one'), tuple(2, 'two')][$this->idx];
  }

  public function next(): void {
    $this->idx++;
  }

  public function rewind(): void {
    $this->idx = 0;
  }

  public function valid(): bool {
    return $this->idx < 2;
  }
}

<<__EntryPoint>>
function main(): void {
  echo "=== shape key, tuple value ===\n";
  foreach (new ShapeKeyedIterator() as shape('id' => $id, 'tag' => $tag) => tuple($num, $name)) {
    echo "id=$id tag=$tag num=$num name=$name\n";
  }

  echo "=== tuple key, shape value ===\n";
  foreach (new TupleKeyedIterator() as tuple($id, $label) => shape('ok' => $ok)) {
    $ok_str = $ok ? 'true' : 'false';
    echo "id=$id label=$label ok=$ok_str\n";
  }
}
