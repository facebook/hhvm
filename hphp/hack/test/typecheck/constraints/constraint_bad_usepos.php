<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I { }

class C<TC as I> {
  public function __construct(private TC $item) { }
}

class D<TD>
// Should report error here, not at definition of constraint
  extends C<TD>
  { }
