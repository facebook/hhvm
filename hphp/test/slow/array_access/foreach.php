<?php
class C implements ArrayAccess {
  public $arr = array();
  public function offsetGet($k) {
    echo "C::offsetGet $k\n";
    return $this->arr[$k];
  }
  public function offsetSet($k, $v) { $this->arr[$k] = $v; }
    public function offsetExists($k) { return isset($this->arr[$k]); }
    public function offsetUnset($k) { unset($this->arr[$k]); }
}
class D implements ArrayAccess {
  public $arr = array();
  public function __construct($x, $y) {
    $this->arr[0] = $x;
    $this->arr[1] = $y;
  }
  public function offsetGet($k) {
    global $a, $b, $c, $d;
    echo "D::offsetGet $k\n";
    echo "  a=$a b=$b c=$c d=$d\n";
    return $this->arr[$k];
  }
  public function offsetSet($k, $v) { $this->arr[$k] = $v; }
    public function offsetExists($k) { return isset($this->arr[$k]); }
    public function offsetUnset($k) { unset($this->arr[$k]); }
}
$x = new C;
$x->arr[0] = new D(11, 22);
$x->arr[1] = new D(33, 44);
$a = $b = $c = $d = 0;
foreach (array($x) as list(list($a,$b),list($c,$d))) {}
