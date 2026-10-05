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

function shape_key_accesses(
  AnnotatedType $marked,
  MissingAttributeType $unmarked,
): void {
  $_ = $marked[AnnotatedShapeKey::KEY];
  $_ = $unmarked[MissingShapeKey::KEY];
}
