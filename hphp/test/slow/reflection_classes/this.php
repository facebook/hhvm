<?hh
function printFunc($rf) {
  $rps = $rf->getParameters();
  foreach($rps as $rp) {
    var_dump($rp->getTypeText());
  }
  var_dump($rf->getReturnTypeText());
}
function printClass($rc) {
  $rms = $rc->getMethods();
  $meths = array();
  foreach($rms as $rm) {
    $meths[$rm->getName()] = $rm;
  }
  ksort($meths);
  foreach($meths as $meth) {
    printFunc($meth);
  }
  $rps = $rc->getProperties();
  $props = array();
  foreach($rps as $rp) {
    $props[$rp->getName()] = $rp;
  }
  ksort($props);
  foreach($props as $prop) {
    var_dump($prop->getTypeText());
  }
}

class C {
  public function m() : this { return $this; }
}

$rc = new ReflectionClass('C');
printClass($rc);
