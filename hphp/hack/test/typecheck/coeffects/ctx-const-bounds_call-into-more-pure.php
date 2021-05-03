<?hh

abstract class Base {
  abstract const ctx C as [globals];

  public function m()[this::C]: void {
    require_rx(); // OK: this::C <: [globals]
  }
}

function require_rx()[globals]: void {}

abstract class Child extends Base {
  abstract const ctx C as [defaults]; // OK: [defaults] <: [globals]

  <<__Override>>
  public function m()[this::C]: void {
    require_defaults(); // OK: this::C <: [defaults]
  }
}

function require_defaults(): void {}
