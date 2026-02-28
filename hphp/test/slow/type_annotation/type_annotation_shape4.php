<?hh

class D {
  const float F1 = 3.4;
}

class C {
  const type T = shape(D::F1=>bool);
}
<<__EntryPoint>> function main(): void {
$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getTypeStructure());
}
