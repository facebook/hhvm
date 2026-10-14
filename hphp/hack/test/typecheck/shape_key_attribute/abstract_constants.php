<?hh

abstract class AbstractShapeKeyConstants {
  <<__ShapeKey>>
  abstract const arraykey KEY;

  <<__ShapeKey>>
  abstract const string DEFAULT_KEY = 'default';

  abstract const type T as arraykey;

  <<__ShapeKey>>
  abstract const this::T TYPE_CONSTANT;
}

interface AbstractInterfaceShapeKeyConstants {
  <<__ShapeKey>>
  abstract const int KEY;
}

trait AbstractTraitShapeKeyConstants {
  <<__ShapeKey>>
  abstract const string DEFAULT_KEY = 'default';
}
