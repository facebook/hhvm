<?hh

/** I's doc comment */
interface I {
  /** @idx herp derp */
  public function meth($idx):mixed;
}

/** C's doc comment */
class C {
  /** doc comment */
  public $p1;
  /* not doc comment */
  private $p2;

  /** more doc comment */
  public function okay() :mixed{ }
}


<<__EntryPoint>>
function main_get_doc_comment_builtins() :mixed{
$meths = varray[
  varray['ArrayAccess', 'offsetExists'],
  varray['ReflectionMethod', 'getDocComment'],
  varray['I', 'meth'],
  varray['C', 'okay']
];

foreach ($meths as list($class, $meth)) {
  $refl = new ReflectionMethod($class, $meth);
  $s = $refl->getDocComment();
  var_dump($s);
  $s = $refl->getDeclaringClass()->getDocComment();
  var_dump($s);
}

foreach (varray['p1', 'p2'] as $prop) {
  $refl = new ReflectionProperty('C', $prop);
  $s = $refl->getDocComment();
  var_dump($s);
}
}
