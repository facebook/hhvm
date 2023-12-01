<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public ?darray<string,int> $da;
  public function testit(?string $so):void {
    $this->da ??= dict[];
    /* HH_FIXME[4110] */
    $this->da[$so] = 3;
  }
}
