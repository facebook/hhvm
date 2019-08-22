<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I { }

class C<TC as I> {
  public function __construct(private TC $item) { }
}

class D<TD>
  // This fixme silences the error from this line
  // It should not leave a phantom error at the definition site!
  /* HH_FIXME[4323] */
  /* HH_FIXME[4110] */
  extends C<TD>
  { }
