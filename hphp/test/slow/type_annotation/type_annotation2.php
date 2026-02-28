<?hh

class C {
  const type T = Vector<?int>;

  static public function check<Ts>(TypeStructure<Ts> $other) : bool {
    $info = type_structure(__CLASS__, 'T');
    return ($info === $other);
  }
}


<<__EntryPoint>>
function main_type_annotation2() :mixed{
$info1 = type_structure(C::class, 'T');
$info2 = type_structure(new C, 'T');

var_dump($info1 === $info2);
var_dump(C::check($info1));

var_dump($info1);
}
