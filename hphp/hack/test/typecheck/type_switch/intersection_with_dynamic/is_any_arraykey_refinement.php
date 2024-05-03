<?hh

function mymap<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}

final class Foo {
  public function getShape(): shape(
    ?'columns' => vec<shape('name' => string)>,
  ) {
    return shape();
  }

  private function do(): void {
    $shape = $this->getShape();

    $columns = Shapes::idx($shape, 'columns');
    if ($columns is null) {
      return;
    }
    invariant(HH\is_any_array($columns), '');
    $column_names = mymap($columns, $col ==> {
      return $col['name'];
    });
  }
}
