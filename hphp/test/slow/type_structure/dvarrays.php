<?hh

class C {
  const type D_ = darray<string, bool>;
  const type V_ = varray<int>;
  const type DV = varray_or_darray<float>;
  const type SH = shape('a' => int, 'b' => string);
  const type TU = (int, string);

  public function d_(): darray<string, bool> { return dict[]; }
  public function v_(): varray<int> { return vec[]; }
  public function dv(): varray_or_darray<float> { return vec[]; }
  public function sh(): shape(?'a' => int, ?'b' => string) { return shape(); }
  public function tu(): (int, string) { return tuple(17, 'x'); }
}

function show($x) :mixed{
  print(json_encode($x, JSON_FB_FORCE_HACK_ARRAYS)."\n");
}

<<__EntryPoint>>
function main() :mixed{
  print("type_structure:\n");
  show(type_structure(C::class, 'D_'));
  show(type_structure(C::class, 'V_'));
  show(type_structure(C::class, 'DV'));
  show(type_structure(C::class, 'SH'));
  show(type_structure(C::class, 'TU'));

  $cls = new ReflectionClass('C');
  print("\nReflection: Methods:\n");
  foreach ($cls->getMethods() as $method) {
    print($method->getName().': '.$method->getReturnTypeText()."\n");
  }
  print("\nReflection: TypeConstants:\n");
  foreach ($cls->getTypeConstants() as $cns) {
    print($cns->getName().': '.$cns->getAssignedTypeText()." =>  ");
    show($cns->getTypeStructure());
  }
}
