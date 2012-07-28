<?php

class Base {
  public $basePub = 10;
  protected $baseProt = 20;
  private $basePriv = 30;

  public function baseIterate() {
    foreach ($this as $k => $v) {
      echo "$k => $v\n";
    }
  }
}

class Child extends Base {
  public $childPub = 40;
  protected $childProt = 50;
  private $childPriv = 60;

  // Same name as private in base class, different value
  private $basePriv = "child's basePriv";

  public function childIterate() {
    foreach ($this as $k => $v) {
      echo "$k => $v\n";
    }
  }
}

$b = new Base();
$b->baseDynamic = "base dynamic";
echo "\nBase context, Base object\n";
$b->baseIterate();

echo "\nAnonymous context, Base object\n";
foreach ($b as $k => $v) {
  echo "$k => $v\n";
}

$c = new Child();
$c->childDynamic = "child dynamic";

echo "\nChild context, Child object\n";
$c->childIterate();
echo "\nBase context, Child object\n";
$c->baseIterate();

echo "\nAnonymous context, Child object\n";
foreach ($c as $k => $v) {
  echo "$k => $v\n";
}

$c = new Child();
$c->dynamic = "dynamic";
echo "\nAnonymous context, Child object, strong foreach\n";
foreach ($c as $k => &$v) {
  $v = "BLARK";
}
var_dump($c);

// empty iteration
$c = new stdclass();
foreach ($c as $k => $v) {
  echo "empty object has properties, oh no\n";
}
foreach ($c as $k => &$v) {
  echo "empty object (strong) has properties, oh no\n";
}