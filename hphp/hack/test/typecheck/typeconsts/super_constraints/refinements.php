<?hh

class One {}
class Two extends One {}
class Three extends Two {}

abstract class Foo {
  abstract const type Ta as One super Two;
}

function ok(Foo with { type Ta as One super Two } $x): void {}

function redundant(Foo with { type Ta as One super Three } $x): void {}

function also_redundant(Foo with { type Ta as One } $x): void {}

function nope(Foo with { type Ta super One } $x): void {}

function test(Foo $x): void {
  nope($x);
}
