<?hh

class InvalidShapeKeyTypeWithCheckDisabled {
  <<__ShapeKey>>
  const float KEY = 1.5;
}

abstract class AbstractShapeKeyWithCheckDisabled {
  <<__ShapeKey>>
  abstract const arraykey KEY;
}
