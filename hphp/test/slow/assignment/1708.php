<?php

class dom {
  public $kid;
  public function __destruct() {
    echo "dom destructing
";
    $this->kid->check();
    $this->kid->clear_unset();
  }
}

class node {
  public $dom;

  public function __construct($dom) {
    $this->dom = $dom;
    $dom->kid = $this;
  }
  public function __destruct() {
    echo "node destructing
";
  }
  public function clear_unset() {
    unset($this->dom);
  }
  public function clear_set() {
    $this->dom = null;
  }
  public function check() {
    var_dump(isset($this->dom));
  }
}

class node_arr {
  public $doms = array();

  public function __construct($dom) {
    $this->doms[0] = $dom;
    $dom->kid = $this;
  }
  public function __destruct() {
    echo "node destructing
";
  }
  public function clear_unset() {
    unset($this->doms[0]);
  }
  public function clear_set() {
    $this->doms[0] = null;
  }
  public function check() {
    var_dump(isset($this->doms[0]));
  }
}

echo "
Property, SetM
";
$node = new node(new dom);
$node->clear_set();
unset($node);

echo "
Property, UnsetM
";
$node = new node(new dom);
$node->clear_unset();
unset($node);

echo "
Array, SetM
";
$node = new node_arr(new dom);
$node->clear_set();
unset($node);

echo "
Array, UnsetM
";
$node = new node_arr(new dom);
$node->clear_unset();
unset($node);
