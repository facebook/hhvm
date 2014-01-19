<?php

/* Compile only test. Used to crash hphp */ class X {
  protected $map;
  protected $parents;
  public function __construct(array $map, array $parents) {
    $this->map = $map;
    $this->parents = $parents;
  }
}
