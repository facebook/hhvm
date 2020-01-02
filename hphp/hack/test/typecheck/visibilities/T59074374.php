<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  protected function __construct(){}
}

class B extends A {
  public function __construct(){
    parent::__construct();
  }
}

interface I {
  require extends B;
}

class C extends B implements I {

}

trait T implements I {

}

class D extends B {
  use T;
}
