<?hh

trait TraitShapeKey {
  <<__ShapeKey>>
  const string KEY = 'trait';
  const string UNMARKED = 'unmarked';
}

class TraitShapeKeyUser {
  use TraitShapeKey;
}

type TraitType = shape(TraitShapeKeyUser::KEY => int);
type TraitMissingType = shape(TraitShapeKeyUser::UNMARKED => int);

function shape_key_expressions(): void {
  $_ = shape(TraitShapeKeyUser::KEY => 1);
  $_ = shape(TraitShapeKeyUser::UNMARKED => 1);
}
