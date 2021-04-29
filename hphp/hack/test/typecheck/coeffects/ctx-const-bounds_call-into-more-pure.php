<?hh

abstract class Base {
  abstract const ctx C as [rx];

  public function m()[this::C]: void {
    require_rx(); // OK: this::C <: [rx]
  }
}

function require_rx()[rx]: void {}

abstract class Child extends Base {
  abstract const ctx C as [defaults]; // OK: [defaults] <: [rx]

  <<__Override>>
  public function m()[this::C]: void {
    require_defaults(); // OK: this::C <: [defaults]
  }
}

function require_defaults(): void {}
