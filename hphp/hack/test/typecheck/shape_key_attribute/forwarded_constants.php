<?hh

class AnnotatedShapeKey {
  <<__ShapeKey>>
  const string KEY = 'annotated';
}

class ForwardedResolvableShapeKey {
  <<__ShapeKey>>
  const string KEY = AnnotatedShapeKey::KEY;
  const string UNMARKED = AnnotatedShapeKey::KEY;
}

type ForwardedType = shape(ForwardedResolvableShapeKey::KEY => int);
type ForwardedMissingType =
  shape(ForwardedResolvableShapeKey::UNMARKED => int);

function shape_key_expressions(): void {
  $_ = shape(ForwardedResolvableShapeKey::KEY => 1);
  $_ = shape(ForwardedResolvableShapeKey::UNMARKED => 1);
}
