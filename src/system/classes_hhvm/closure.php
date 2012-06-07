<?php

// Used as the base class for all closures
class Closure {
  protected $__static_locals;
  // Adding a dummy __sleep() to return an illegal value to make the code
  // go through error handling path
  public function __sleep() {
    return false;
  }
}

// Unused, but needed for consistency with class_map.cpp
class GeneratorClosure extends Closure {
}
