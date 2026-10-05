<?hh

class ParentShapeKey {
  <<__ShapeKey>>
  const string KEY = 'inherited';
  const string UNMARKED = 'unmarked';

  public static function shape_accesses(shape(...) $s): void {
    $_ = Shapes::idx($s, self::KEY);
    $_ = Shapes::idx($s, self::UNMARKED);
    $_ = Shapes::idx($s, nameof self);
  }
}
