<?hh

class MissingShapeKey {
  const string KEY = 'missing';
}

class AnnotatedShapeKey {
  <<__ShapeKey>>
  const string KEY = 'annotated';
}

type AnnotatedType = shape(AnnotatedShapeKey::KEY => int);
type MissingAttributeType = shape(MissingShapeKey::KEY => int);

function shape_key_expressions(): void {
  $_ = shape(AnnotatedShapeKey::KEY => 1);
  $_ = shape(MissingShapeKey::KEY => 1);
}
