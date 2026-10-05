<?hh

interface InterfaceShapeKey {
  <<__ShapeKey>>
  const string KEY = 'interface';
  const string UNMARKED = 'unmarked';
}

type InterfaceType = shape(InterfaceShapeKey::KEY => int);
type InterfaceMissingType = shape(InterfaceShapeKey::UNMARKED => int);

function shape_key_expressions(): void {
  $_ = shape(InterfaceShapeKey::KEY => 1);
  $_ = shape(InterfaceShapeKey::UNMARKED => 1);
}
