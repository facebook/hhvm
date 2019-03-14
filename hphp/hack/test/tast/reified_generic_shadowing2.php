<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Tc<Tc> {
  // Tc here comes from the type parameter
  public function f(Tc<int> $x): void {}
}

class Tc_other {
  // Tc here comes from the class
  public function f(Tc<int> $x): void {}
}
