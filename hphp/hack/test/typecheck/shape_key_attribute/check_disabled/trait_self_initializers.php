<?hh

trait TraitSelfShapeKeyInitializersWithCheckDisabled {
  const string LOCAL_KEY = 'local';

  <<__ShapeKey>>
  const string SELF_CONSTANT = self::LOCAL_KEY;

  <<__ShapeKey>>
  const string SELF_CLASS_NAME = nameof self;
}
