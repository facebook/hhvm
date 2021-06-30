<?hh

abstract class Base {
  abstract const ctx C;
  public abstract function m()[this::C]: void;
}

function bad_mono_caller_implicit(Base $b): void {
  $b->m(); // ERROR (must not implicitly hold: [defaults] <: C)
}

function poly_caller_implicit(Base $b)[$b::C]: void {
  $b->m(); // OK
}
