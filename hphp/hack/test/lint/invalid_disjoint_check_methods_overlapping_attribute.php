<?hh

/* INSTANCE METHODS */
class C {
  <<__Overlapping('T1', 'T2')>>
  public function contains<T1, T2>(
    Traversable<T1> $_t,
    T2 $_v,
  ): bool {
    return false;
  }

  public function shouldDeleteAssoc(
    dict<int, Map<string, mixed>> $table_data,
    Map<string, mixed> $assoc,
  ): bool {
    return !$this->contains($table_data, (int)$assoc['id1']);
  }
}

final class Foo {}

class Bar {}
final class Baz extends Bar {}

function bad1(C $c, vec<Foo> $x, int $i): bool {
  return $c->contains($x, $i);
}
function bad2(C $c, dict<int, Foo> $x, int $i): bool {
  return $c->contains($x, $i);
}
function bad3(C $c, Traversable<Foo> $x, int $i): bool {
  return $c->contains($x, $i);
}
function bad4(C $c, KeyedTraversable<int, Foo> $x, int $i): bool {
  return $c->contains($x, $i);
}
function bad5(C $c, Container<Foo> $x, int $i): bool {
  return $c->contains($x, $i);
}
function bad6(C $c, KeyedContainer<int, Foo> $x, int $i): bool {
  return $c->contains($x, $i);
}
function bad7(C $c, Iterator<Foo> $x, int $i): bool {
  return $c->contains($x, $i);
}
function bad8(C $c, IteratorAggregate<Foo> $x, int $i): bool {
  return $c->contains($x, $i);
}
function bad9(C $c, vec<Foo> $x, Baz $i): bool {
  return $c->contains($x, $i);
}
function bad10(C $c, vec<Foo> $x, Bar $i): bool {
  return $c->contains($x, $i);
}
function bad11(C $c, vec<Bar> $x, Foo $i): bool {
  return $c->contains($x, $i);
}

function good1(C $c, vec<Foo> $x, Foo $i): bool {
  return $c->contains($x, $i);
}
function good2(C $c, vec<Bar> $x, Baz $i): bool {
  return $c->contains($x, $i);
}
function good3(C $c, vec<Baz> $x, Bar $i): bool {
  return $c->contains($x, $i);
}

enum OpaqueEnumInt : int {
  OFOO = 1;
}
enum OpaqueEnumString : string {
  OBAR = 'a';
}
enum EnumInt : int as int {
  FOO = 1;
}
enum EnumString : string as string {
  BAR = 'a';
}

// These are handled differently by the linter_equality_check
function goodish1(C $c, vec<int> $x, OpaqueEnumInt $y): bool {
  return $c->contains($x, $y);
}

function goodish2(C $c, vec<string> $x, OpaqueEnumInt $y): bool {
  return $c->contains($x, $y);
}

function goodish3(C $c, vec<int> $x, EnumInt $y): bool {
  return $c->contains($x, $y);
}

function goodish4(C $c, vec<OpaqueEnumInt> $x, OpaqueEnumInt $y): bool {
  return $c->contains($x, $y);
}

function goodish5(C $c, vec<OpaqueEnumInt> $x, OpaqueEnumString $y): bool {
  return $c->contains($x, $y);
}

function badenum(C $c, vec<string> $x, EnumInt $y): bool {
  return $c->contains($x, $y);
}

class ExpectObj<T> {
  public function __construct(private T $item) { }
  <<__Overlapping('T', 'T2')>>
  public function isEqual<T2>(T2 $other):bool {
    return $this->item === $other;
  }
}
// Need to check that inheritance works as expected
class ExpectObjString extends ExpectObj<string> {
}
class ExpectObjVec<T> extends ExpectObj<vec<T>> {
}

function expect<T>(T $x):ExpectObj<T> {
  return new ExpectObj($x);
}
function test_expect(dynamic $d):void {
  // Should be accepted
  expect(3)->isEqual(4);
  // Should be rejected
  expect(3)->isEqual("A");
  // Should be accepted; we don't want to warn on ~null
  $x = Shapes::idx($d[0], 'x');
  expect($x)->isEqual('A');
  // These should behave the same, i.e. reject
  $x = new ExpectObj<string>();
  $x->isEqual(3);
  $y = new ExpectObjString();
  $y->isEqual(3);
  $z = new ExpectObjVec<int>();
  // Should be accepted
  $z->isEqual(vec[2]);
  // Should be rejected
  $z->isEqual(false);
}
