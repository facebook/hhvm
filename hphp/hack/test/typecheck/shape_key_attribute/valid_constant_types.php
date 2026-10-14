<?hh

type StringShapeKey = string;
newtype IntShapeKey as int = int;

enum EnumShapeKeyType: string {
  KEY = 'enum';
}

class ValidShapeKeyTypes {
  <<__ShapeKey>>
  const int INT = 1;

  <<__ShapeKey>>
  const string STRING = 'key';

  <<__ShapeKey>>
  const arraykey ARRAYKEY = 'key';

  <<__ShapeKey>>
  const StringShapeKey ALIAS = 'alias';

  <<__ShapeKey>>
  const IntShapeKey NEWTYPE = 2;

  <<__ShapeKey>>
  const EnumShapeKeyType ENUM = EnumShapeKeyType::KEY;

  <<__ShapeKey>>
  const classname<ValidShapeKeyTypes> CLASSNAME = ValidShapeKeyTypes::class;

  <<__ShapeKey>>
  const INFERRED_INT = 3;

  <<__ShapeKey>>
  const INFERRED_STRING = 'inferred';
}

abstract class ValidAbstractShapeKeyTypes {
  <<__ShapeKey>>
  const string KEY = 'concrete';
}

trait ValidTraitShapeKeyInitializers {
  const string UNANNOTATED_SELF_CLASS_NAME = nameof self;

  <<__ShapeKey>>
  const string LITERAL = 'literal';

  <<__ShapeKey>>
  const string PINNED_CLASS_NAME = nameof ValidTraitShapeKeyInitializers;
}

class ValidClassSelfShapeKeyInitializer {
  <<__ShapeKey>>
  const string SELF_CLASS_NAME = nameof self;
}
