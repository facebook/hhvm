<?hh

class InvalidShapeKeyTypes {
  <<__ShapeKey>>
  const vec<int> VEC = vec[1];

  <<__ShapeKey>>
  const dict<string, int> DICT = dict['key' => 1];

  <<__ShapeKey>>
  const keyset<string> KEYSET = keyset['key'];

  <<__ShapeKey>>
  const float FLOAT = 1.5;

  <<__ShapeKey>>
  const bool BOOL = true;

  <<__ShapeKey>>
  const null NULL_KEY = null;

  <<__ShapeKey>>
  const ?string NULLABLE = 'key';

  <<__ShapeKey>>
  const num NUM = 1;

  <<__ShapeKey>>
  const mixed MIXED = 'key';

  <<__ShapeKey>>
  const dynamic DYNAMIC = 'key';

  <<__ShapeKey>>
  const INFERRED_FLOAT = 1.5;

  <<__ShapeKey>>
  const INFERRED_BOOL = true;

  <<__ShapeKey>>
  const INFERRED_NULL = null;
}

interface InvalidInterfaceShapeKeyTypes {
  <<__ShapeKey>>
  const float FLOAT = 1.5;
}

trait InvalidTraitShapeKeyTypes {
  <<__ShapeKey>>
  const bool BOOL = false;
}
