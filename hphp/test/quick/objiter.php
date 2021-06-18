<?hh

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
<<__EntryPoint>> function main(): void {
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

// empty iteration
$c = new stdClass();
foreach ($c as $k => $v) {
  echo "empty object has properties, oh no\n";
}
}
