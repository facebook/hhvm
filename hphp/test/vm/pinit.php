<?hh

// This is to trigger a specific refcounting bug in the VM, to do with object
// initialization templates. #668149

class Fub {
  const FLUB = 'flub';
  // This depends on a class constant, which means it will need a 86pinit().
  public $dub = darray[
    self::FLUB => varray[123]
  ];
}

$f = new Fub;
var_dump($f->dub);
unset($f);

$f = new Fub;
var_dump($f->dub);
