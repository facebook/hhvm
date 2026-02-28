<?hh

final class WithConstantName {
  const string FLD1 = 'field1';

  public function self_inside_class(): shape(self::FLD1 => int) {
    return shape(self::FLD1 => 1);
  }
}

type with_class_constant = shape(WithConstantName::FLD1 => int);

type with_self_outside_class = shape(self::FLD1 => int);

type with_string_name = shape("field1" => int);

// Note this is currently a parse error but not a naming error
type with_int_name = shape(1 => int);
