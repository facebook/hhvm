<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B<T> {
  public function __construct() {}
}

class C<T> {
  public function __construct<Tc>() {}
}
