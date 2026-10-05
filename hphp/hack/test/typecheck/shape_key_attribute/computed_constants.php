<?hh

class UnresolvableShapeKey {
  <<__ShapeKey>>
  const string KEY = 'not'.'statically resolvable';
  const string UNMARKED = 'also not'.'statically resolvable';
}

type ComputedType = shape(UnresolvableShapeKey::KEY => int);
type ComputedMissingType = shape(UnresolvableShapeKey::UNMARKED => int);

function shape_key_expressions(): void {
  $_ = shape(UnresolvableShapeKey::KEY => 1);
  $_ = shape(UnresolvableShapeKey::UNMARKED => 1);
}
