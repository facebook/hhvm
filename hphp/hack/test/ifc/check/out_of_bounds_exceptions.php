<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  <<__InferFlows>>
  public function __construct(<<__Policied("PRIVATE")>> public int $valuex) {}
}

class Y {
  <<__InferFlows>>
  public function __construct(<<__Policied("PUBLIC")>> public int $valuey) {}
}

<<__InferFlows>>
function koVecAccess(X $x, Y $y, vec<int> $v): void {
  if ($x->valuex > 10) {
    // The following may throw out of bounds exception.
    // Should conservatively behave same as throw.
    $v[0];
  }
  $y->valuey = 10;
}

<<__InferFlows>>
function koVecAssign(X $x, Y $y, vec<int> $v): void {
  if ($x->valuex > 10) {
    // The following may throw out of bounds exception.
    // Should conservatively behave same as throw.
    $v[0] = 42;
  }
  $y->valuey = 10;
}

<<__InferFlows>>
function okDictAssign(X $x, Y $y, dict<int,int> $dict): void {
  // Assigning to a dictionary does not cause an exception to be thrown, so
  // there is no leak here.
  if ($x->valuex > 10) {
    $dict[0] = 42;
  }
  $y->valuey = 10;
}

<<__InferFlows>>
function koDictAccess(X $x, Y $y, dict<string,int> $v): void {
  if ($x->valuex > 10) {
    // The following may throw out of bounds exception.
    // Should conservatively behave same as throw.
    $v['hi'];
  }
  $y->valuey = 10;
}

<<__InferFlows>>
function okDict(X $x, Y $y, dict<string,int> $v): void {
  if ($x->valuex > 10) {
    // The following is fine because we handle all exceptions.
    try {
      $v['hi'];
    } catch (Exception $_) { }
  }
  $y->valuey = 10;
}

class V {
  <<__Policied("VEC")>>
  public vec<string> $vec = vec[];

  <<__Policied("PUBLIC")>>
  public bool $b = false;

  <<__InferFlows>>
  public function koIsEmptyAccess(): void {
    if (isEmptyAccess($this->vec)) {
      $this->b = true; // VEC leaks to PUBLIC through length
    }
  }

  <<__InferFlows>>
  public function koIsEmptyAssign(): void {
    if (isEmptyAssign($this->vec)) {
      $this->b = true; // VEC leaks to PUBLIC through length
    }
  }
}

// The following decides the length using exceptions.
<<__InferFlows>>
function isEmptyAccess(vec<string> $v): bool {
  try {
    $v[1];
    return false; // Leak goes through here
  } catch (Exception $_) {
    return true; // Leak also goes through here
  }
}

// The following decides the length using exceptions.
<<__InferFlows>>
function isEmptyAssign(vec<string> $v): bool {
  try {
    $v[1] = 42;
    return false; // Leak goes through here
  } catch (Exception $_) {
    return true; // Leak also goes through here
  }
}
