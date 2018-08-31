<?php


<<__EntryPoint>>
function main_anon_class_with_anon_fn_params() {
$c = new class(function($a, $b) { return $a + $b; }, function($a) { return $a; }) {
  public $f;
  public $g;
  public function __construct($f, $g) {
    $this->f = $f;
    $this->g = $g;
  }
};

$f = $c->f;
$g = $c->g;
var_dump($f(1, 2) + $g(4));
}
