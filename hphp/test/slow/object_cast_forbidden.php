<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Nope {
  public function cast(mixed $value): mixed{
    return (object)$value;
  }
}
