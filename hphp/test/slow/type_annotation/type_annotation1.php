<?hh // strict
class C {
  const type T = ?array<int, @bool>;
  const type U = map<arraykey, Vector<array<int>>>;
  const type V = (int, this, ?float, foo);
  const type W = (function (): void);
  const type X = (function (mixed, resource): array);
  const type Y = N::M::O::P::Q;
  const type Z = this::T;
}

$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'U');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'V');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'W');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'X');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'Y');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'Z');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());
