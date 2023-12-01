<?hh

class MyContainer<Tv> {
  public function setPair(Pair<string, Tv> $_): void {}
  public function setTuple((string, Tv) $_): void {}
  public function setVector(Vector<Tv> $_): void {}
  public function setMap(Map<string, Tv> $_): void {}
  public function setMapArray(darray<string, Tv> $_): void {}
  public function setVectorArray(varray<Tv> $_): void {}
  public function setShape(shape('x' => Tv) $_): void {}
}

function take_int(int $_): void {}

function testPair(): void {
  $x = Vector { 4 };
  $y = Vector { 'zzz' };

  $m = new MyContainer();

  // $x[0] and $y[0] are type variables that contain inferred types of elements
  // in $x and $y - Pair constructor must remove (unbind) them, because in next
  // two lines they would be unified with Tv type variable of $m, and
  // transitively with each other. Putting element inside a container should
  // not affect it's type.
  $m->setPair(Pair { 'x', $x[0] });
  $m->setPair(Pair { 'x', $y[0] });

  take_int($x[0]);
}

function testTuple(): void {
  $x = Vector { 4 };
  $y = Vector { 'zzz' };

  $m = new MyContainer();

  $m->setTuple(tuple('x', $x[0]));
  $m->setTuple(tuple('x', $y[0]));

  take_int($x[0]);
}

function testVector(): void {
  $x = Vector { 4 };
  $y = Vector { 'zzz' };

  $m = new MyContainer();

  $m->setVector(Vector { $x[0] });
  $m->setVector(Vector { $y[0] });

  take_int($x[0]);
}

function testMap(): void {
  $x = Vector { 4 };
  $y = Vector { 'zzz' };

  $m = new MyContainer();

  $m->setMap(Map { 'x' => $x[0] });
  $m->setMap(Map { 'x' => $y[0] });

  take_int($x[0]);
}

function testMapArray(string $key): void {
  $x = Vector { 4 };
  $y = Vector { 'zzz' };

  $m = new MyContainer();

  $m->setMapArray(dict[$key => $x[0]]);
  $m->setMapArray(dict[$key => $y[0]]);

  take_int($x[0]);
}

function testVectorArray(): void {
  $x = Vector { 4 };
  $y = Vector { 'zzz' };

  $m = new MyContainer();

  $m->setVectorArray(vec[$x[0]]);
  $m->setVectorArray(vec[$y[0]]);

  take_int($x[0]);
}

function testShape(): void {
  $x = Vector { 4 };
  $y = Vector { 'zzz' };

  $m = new MyContainer();

  $m->setShape(shape('x' => $x[0]));
  $m->setShape(shape('x' => $y[0]));

  take_int($x[0]);
}

function testShapeLikeArray(): void {
  $x = Vector { 4 };
  $y = Vector { 'zzz' };

  $m = new MyContainer();

  $m->setMapArray(dict['x' => $x[0]]);
  $m->setMapArray(dict['x' => $y[0]]);

  take_int($x[0]);
}
