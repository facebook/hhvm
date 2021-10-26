<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class C {
  public function __construct(private int $x) { }

  // Strong signature
  public function makeAddCopy(~int $y): C {
    // Hack shows an error here, but not if we precede it with
    //   $y as int;
    // This should not be necessary
    $c = new C($this->x + $y);
    return $c;
  }
}
