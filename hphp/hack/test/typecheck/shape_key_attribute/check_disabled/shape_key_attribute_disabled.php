<?hh

class ShapeKeyDisabled {
  const string UNMARKED = 'unmarked';

  <<__ShapeKey>>
  const string COMPUTED = 'computed'.'key';
}

type ShapeKeyDisabledType = shape(ShapeKeyDisabled::UNMARKED => int);

function shape_key_accesses(ShapeKeyDisabledType $s): void {
  $_ = $s[ShapeKeyDisabled::UNMARKED];
  $s[ShapeKeyDisabled::UNMARKED] = 1;
  $_ = Shapes::idx($s, ShapeKeyDisabled::UNMARKED);
  $_ = Shapes::idx($s, ShapeKeyDisabled::UNMARKED, 0);
  $_ = Shapes::at($s, ShapeKeyDisabled::UNMARKED);
  $_ = Shapes::keyExists($s, ShapeKeyDisabled::UNMARKED);
  $_ = Shapes::put($s, ShapeKeyDisabled::UNMARKED, 1);
  Shapes::removeKey(inout $s, ShapeKeyDisabled::UNMARKED);
}
