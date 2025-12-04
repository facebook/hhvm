<?hh

class Huge {}
class Bigly extends Huge {}
class Middling extends Bigly {}
class Smol extends Middling {}
class Tiny extends Smol {}

abstract class Foo {
  abstract const type Ta as Huge super Middling;

  public function returnMiddling(): this::Ta {
    return new Middling();
  }

  public function returnSmol(): this::Ta {
    return new Smol();
  }

  public function returnTiny(): this::Ta {
    return new Tiny();
  }
}
