<?hh

enum EnumShapeKey: string {
  KEY = 'enum';
}

type EnumType = shape(EnumShapeKey::KEY => int);

function exempt_shape_key_accesses(EnumType $enum): void {
  $_ = shape(EnumShapeKey::KEY => 1);
  $_ = Shapes::idx($enum, EnumShapeKey::KEY);
}
