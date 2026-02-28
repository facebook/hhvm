<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_string(?string $s): void {}

function repro(?string $b): void {
  $l = async () ==> {
    if ($b === null) {
      $b = 4;
    }
    expect_string($b); // error, $b can be int
  };
}

function no_repro(): void {
  $b = "foo";
  $l = async () ==> {
    if ($b === null) {
      $b = 4;
    }
    expect_string($b); // error, $b can be int
  };
}
