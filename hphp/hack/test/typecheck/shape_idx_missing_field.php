<?hh

function type_hole<T>(shape(...) $s): ?T {
  return Shapes::idx($s, 'x');
}
