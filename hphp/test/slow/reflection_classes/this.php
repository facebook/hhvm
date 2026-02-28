<?hh
function printFunc($rf) :mixed{
  $rps = $rf->getParameters();
  foreach($rps as $rp) {
    var_dump($rp->getTypeText());
  }
  var_dump($rf->getReturnTypeText());
}
function printClass($rc) :mixed{
  $rms = $rc->getMethods();
  $meths = dict[];
  foreach($rms as $rm) {
    $meths[$rm->getName()] = $rm;
  }
  ksort(inout $meths);
  foreach($meths as $meth) {
    printFunc($meth);
  }
  $rps = $rc->getProperties();
  $props = dict[];
  foreach($rps as $rp) {
    $props[$rp->getName()] = $rp;
  }
  ksort(inout $props);
  foreach($props as $prop) {
    var_dump($prop->getTypeText());
  }
}

class C {
  public function m() : this { return $this; }
}


<<__EntryPoint>>
function main_this() :mixed{
$rc = new ReflectionClass('C');
printClass($rc);
}
