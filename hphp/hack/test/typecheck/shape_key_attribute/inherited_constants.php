<?hh

class ParentShapeKey {
  <<__ShapeKey>>
  const string KEY = 'inherited';
  const string UNMARKED = 'unmarked';
}

class ChildShapeKey extends ParentShapeKey {}

type InheritedType = shape(ChildShapeKey::KEY => int);
type InheritedMissingType = shape(ChildShapeKey::UNMARKED => int);

function shape_key_expressions(): void {
  $_ = shape(ChildShapeKey::KEY => 1);
  $_ = shape(ChildShapeKey::UNMARKED => 1);
}
