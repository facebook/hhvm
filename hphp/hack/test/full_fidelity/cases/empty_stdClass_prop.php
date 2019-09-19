<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class EmptyStdClassProp {
  public function foo():void{
    $o = new stdClass();
    $o->{''} = true;
  }
}
