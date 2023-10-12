<?hh // strict

function should_delete_assoc(
  dict<int, Map<string, mixed>> $table_data,
  Map<string, mixed> $assoc,
): bool {
  return !HH\Lib\C\contains($table_data, (int)$assoc['id1']);
}

final class Foo {}

class Bar {}
final class Baz extends Bar {}

function bad1(vec<Foo> $x, int $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad2(dict<int, Foo> $x, int $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad3(Traversable<Foo> $x, int $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad4(KeyedTraversable<int, Foo> $x, int $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad5(Container<Foo> $x, int $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad6(KeyedContainer<int, Foo> $x, int $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad7(Iterator<Foo> $x, int $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad8(IteratorAggregate<Foo> $x, int $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad9(vec<Foo> $x, Baz $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad10(vec<Foo> $x, Bar $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function bad11(vec<Bar> $x, Foo $i): bool {
  return HH\Lib\C\contains($x, $i);
}

function good1(vec<Foo> $x, Foo $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function good2(vec<Bar> $x, Baz $i): bool {
  return HH\Lib\C\contains($x, $i);
}
function good3(vec<Baz> $x, Bar $i): bool {
  return HH\Lib\C\contains($x, $i);
}
