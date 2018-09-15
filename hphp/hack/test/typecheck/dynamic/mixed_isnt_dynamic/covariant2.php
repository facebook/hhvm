<?hh // strict

class Foo<-Tcontra, +Tco> {}

function bar(Foo<dynamic, dynamic> $x): void {}

function baz(): void {
  $a = new Foo<dynamic, dynamic>();
  bar($a);

  $b = new Foo<mixed, dynamic>();
  bar($b);

  //now fail because mixed is not a subtype of dynamic,
  //and thus not covariant
  $c = new Foo<dynamic, mixed>();
  bar($c);
}
