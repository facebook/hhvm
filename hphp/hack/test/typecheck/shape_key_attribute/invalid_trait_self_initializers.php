<?hh

// The annotation promises a valid shape key even before any shape uses it.
trait InvalidTraitSelfShapeKeyInitializers {
  const string LOCAL_KEY = 'local';

  <<__ShapeKey>>
  const string SELF_CONSTANT = self::LOCAL_KEY;

  <<__ShapeKey>>
  const string SELF_CLASS_NAME = nameof self;
}
