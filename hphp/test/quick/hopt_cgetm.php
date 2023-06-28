<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

class C {
  public $x;
  public function __construct($val) {
    $this->x = $val;
  }
  public function getX() :mixed{
    return $this->x;
  }
  public function incX() :mixed{
    return $this->x++;
  }
}


function foo($o):mixed{
  echo $o->getX();
  echo "\n";
  echo $o->incX();
  echo "\n";
  echo $o->getX();
  echo "\n";
}
<<__EntryPoint>> function main(): void {
$c = new C(2); // change to 4.0
foo($c);
}
