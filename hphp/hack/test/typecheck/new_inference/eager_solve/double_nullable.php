<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(T $_) {}
  public function get(): ?T {
    return null;
  }
}

function expect_int(int $_): void {}

function test(?int $y): void {
  // new Inv<#1>($y) : Inv<#1>
  //   and ?int <: #1
  // After get, we have
  //   $y : ?#1
  // Now solve for #1
  // Clearly we have to make #1 := ?int
  $y = (new Inv($y))->get();
  if ($y !== null) {
    expect_int($y);
  }
}
