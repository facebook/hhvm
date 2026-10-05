<?hh

interface ShapeKeyInterface {
  <<__ShapeKey>>
  const string KEY = 'interface';
  const string UNMARKED = 'unmarked';
}

trait ShapeKeyTrait {
  <<__ShapeKey>>
  const string KEY = 'trait';
  const string UNMARKED = 'unmarked';
}

class ShapeKeyParent {
  <<__ShapeKey>>
  const string KEY = 'parent';
  const string UNMARKED = 'unmarked';
}

class ShapeKeyChild extends ShapeKeyParent {}

class ShapeKeyTraitUser {
  use ShapeKeyTrait;
}
