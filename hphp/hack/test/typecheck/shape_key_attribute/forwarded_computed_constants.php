<?hh

class UnresolvableShapeKey {
  <<__ShapeKey>>
  const string KEY = 'not'.'statically resolvable';
}

class ForwardedUnresolvableShapeKey {
  <<__ShapeKey>>
  const string KEY = UnresolvableShapeKey::KEY;
  const string UNMARKED = UnresolvableShapeKey::KEY;
}

type ForwardedComputedType = shape(ForwardedUnresolvableShapeKey::KEY => int);
type ForwardedComputedMissingType =
  shape(ForwardedUnresolvableShapeKey::UNMARKED => int);

function shape_key_expressions(): void {
  $_ = shape(ForwardedUnresolvableShapeKey::KEY => 1);
  $_ = shape(ForwardedUnresolvableShapeKey::UNMARKED => 1);
}
