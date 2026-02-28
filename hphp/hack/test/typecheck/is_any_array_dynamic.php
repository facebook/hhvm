<?hh

function expect_dynamic(dynamic $_): void {}

function coerceReified<reify T>(mixed $value): T {
  throw new Exception("A");
}

class Bing {
  const type TPoint = Map<string, mixed>;
  public function test(Map<string, mixed> $data): void {
    $points_mixed = idx($data, 'points');
    invariant(
      $points_mixed is Traversable<_>,
      'points must be Vector, vec or array',
    );
    $points = new Vector($points_mixed);

    foreach ($points as $point) {
      // Take this away and it passes
      if ($point is HH\AnyArray<_, _>) {
      }
      $point = coerceReified<this::TPoint>($point);
    }
  }
}
