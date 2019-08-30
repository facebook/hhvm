<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait T1 {
  private ?int $tutorialID;
}

trait T2 {
  private ?int $tutorialID;
}

class C {
  use T1, T2;
}
