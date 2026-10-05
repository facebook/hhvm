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

function shape_key_idx(
  AnnotatedType $marked,
  MissingAttributeType $unmarked,
): void {
  $_ = Shapes::idx($marked, AnnotatedShapeKey::KEY);
  $_ = Shapes::idx($unmarked, MissingShapeKey::KEY);
}

function shape_key_idx_with_default(
  AnnotatedType $marked,
  MissingAttributeType $unmarked,
): void {
  $_ = Shapes::idx($marked, AnnotatedShapeKey::KEY, 0);
  $_ = Shapes::idx($unmarked, MissingShapeKey::KEY, 0);
}

function shape_key_at(
  AnnotatedType $marked,
  MissingAttributeType $unmarked,
): void {
  $_ = Shapes::at($marked, AnnotatedShapeKey::KEY);
  $_ = Shapes::at($unmarked, MissingShapeKey::KEY);
}

function shape_key_exists(
  AnnotatedType $marked,
  MissingAttributeType $unmarked,
): void {
  $_ = Shapes::keyExists($marked, AnnotatedShapeKey::KEY);
  $_ = Shapes::keyExists($unmarked, MissingShapeKey::KEY);
}

function shape_key_put(
  AnnotatedType $marked,
  MissingAttributeType $unmarked,
): void {
  $_ = Shapes::put($marked, AnnotatedShapeKey::KEY, 1);
  $_ = Shapes::put($unmarked, MissingShapeKey::KEY, 1);
}

function shape_key_remove_key(
  AnnotatedType $marked,
  MissingAttributeType $unmarked,
): void {
  Shapes::removeKey(inout $marked, AnnotatedShapeKey::KEY);
  Shapes::removeKey(inout $unmarked, MissingShapeKey::KEY);
}
