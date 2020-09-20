<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class A {
  private Map $headers;
  public function __construct() {
    $this->headers = Map {};
  }
}

<<__EntryPoint>> function main(): void {
  $a = new A();
  echo "Done\n";
}
