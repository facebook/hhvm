<?php

if (true) {
  class A {
    public function __construct($i, $j, $k) {
      $this->a = $i * $i;
      $this->b = $j * $j;
      $this->c = $k * $k;
    }
    public $a;
    protected $b;
    private $c;
    public $aa = 'aa';
    protected $bb = false;
    private $cc = 1.22;
  }
}
class B extends A {
  public function __construct($i, $j, $k) {
    $this->a = $i + $i;
    $this->b = $j + $j;
    $this->c = $k + $k;
  }
  public $a;
  protected $b;
  private $c;
  public $aa = 'aaa';
  protected $bb = 4;
  private $cc = 1.222;
}
function foo() {
  $obj = new B(1, 2, 3);
  var_dump($obj);
}
foo();
