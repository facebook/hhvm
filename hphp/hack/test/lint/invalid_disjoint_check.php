<?hh

function contains<<<__NonDisjoint>> T1, <<__NonDisjoint>> T2>(Traversable<T1> $_t,
  T2 $_v):bool
{ return false; }

function should_delete_assoc(
  dict<int, Map<string, mixed>> $table_data,
  Map<string, mixed> $assoc,
): bool {
  return !contains($table_data, (int)$assoc['id1']);
}

final class Foo {}

class Bar {}
final class Baz extends Bar {}

function bad1(vec<Foo> $x, int $i): bool {
  return contains($x, $i);
}
function bad2(dict<int, Foo> $x, int $i): bool {
  return contains($x, $i);
}
function bad3(Traversable<Foo> $x, int $i): bool {
  return contains($x, $i);
}
function bad4(KeyedTraversable<int, Foo> $x, int $i): bool {
  return contains($x, $i);
}
function bad5(Container<Foo> $x, int $i): bool {
  return contains($x, $i);
}
function bad6(KeyedContainer<int, Foo> $x, int $i): bool {
  return contains($x, $i);
}
function bad7(Iterator<Foo> $x, int $i): bool {
  return contains($x, $i);
}
function bad8(IteratorAggregate<Foo> $x, int $i): bool {
  return contains($x, $i);
}
function bad9(vec<Foo> $x, Baz $i): bool {
  return contains($x, $i);
}
function bad10(vec<Foo> $x, Bar $i): bool {
  return contains($x, $i);
}
function bad11(vec<Bar> $x, Foo $i): bool {
  return contains($x, $i);
}

function good1(vec<Foo> $x, Foo $i): bool {
  return contains($x, $i);
}
function good2(vec<Bar> $x, Baz $i): bool {
  return contains($x, $i);
}
function good3(vec<Baz> $x, Bar $i): bool {
  return contains($x, $i);
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
function goodish1(vec<int> $x, OpaqueEnumInt $y): bool {
  return contains($x, $y);
}

function goodish2(vec<string> $x, OpaqueEnumInt $y): bool {
  return contains($x, $y);
}

function goodish3(vec<int> $x, EnumInt $y): bool {
  return contains($x, $y);
}

function goodish4(vec<OpaqueEnumInt> $x, OpaqueEnumInt $y): bool {
  return contains($x, $y);
}

function goodish5(vec<OpaqueEnumInt> $x, OpaqueEnumString $y): bool {
  return contains($x, $y);
}

function badenum(vec<string> $x, EnumInt $y): bool {
  return contains($x, $y);
}
