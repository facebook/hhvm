<?php
interface i1 {
  abstract const con;
}
interface i2 {
  abstract const con;
}
class a implements i1, i2 {
  const con = 42;
  public function bar() {
    echo "Fail\n";
  }
  final public function main() {
    return $this->bar();
  }
}
class b extends a {
  public function bar() {
    echo "Pass\n";
  }
}
(new b)->main();
