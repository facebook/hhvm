<?hh
function printClass($rc) {
  $rms = $rc->getMethods();
  $meths = array();
  foreach($rms as $rm) {
    $meths[$rm->getName()] = $rm;
  }
  ksort($meths);
  foreach($meths as $meth) {
	var_dump($meth->getReturnTypeText());
  }
}

class C {
  public function m() : this { return $this; }
}

$rc = new ReflectionClass('C');
printClass($rc);

