<?hh

class Foo<-Tcontra, +Tco> {}

function bar(Foo<mixed, mixed> $x): void {}

function baz(): void {
  $a = new Foo<mixed, mixed>();
  bar($a);

  $b = new Foo<mixed, dynamic>();
  bar($b);

  //now fail because dynamic is not a supertype of mixed,
  //and thus not contravariant
  $c = new Foo<dynamic, mixed>();
  bar($c);
}
