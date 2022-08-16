<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T as mixed;
  abstract public function get() : this::T;
  abstract public function set(this::T $val) : void;
}

function unsupported_bounds<T>(Box with {type T as T} $b): T {
  while (true) {}
}

function unsupported_root<Ta as Box, T>(Ta with {type T = T} $b): T {
  while (true) {}
}
