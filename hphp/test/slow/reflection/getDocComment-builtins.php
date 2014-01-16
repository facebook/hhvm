<?php

/** I's doc comment */
interface I {
  /** @idx herp derp */
  public function meth($idx);
}

/** C's doc comment */
class C {
  /** doc comment */
  public $p1;
  /* not doc comment */
  private $p2;

  /** more doc comment */
  public function okay() { }
}

$meths = array(
  array('ArrayAccess', 'offsetExists'),
  array('ReflectionMethod', 'getDocComment'),
  array('I', 'meth'),
  array('C', 'okay')
);

foreach ($meths as list($class, $meth)) {
  $refl = new ReflectionMethod($class, $meth);
  $s = $refl->getDocComment();
  var_dump($s);
  $s = $refl->getDeclaringClass()->getDocComment();
  var_dump($s);
}

foreach (array('p1', 'p2') as $prop) {
  $refl = new ReflectionProperty('C', $prop);
  $s = $refl->getDocComment();
  var_dump($s);
}
