<?hh

function shapes_idx_into_unknown_field(shape(...) $shape): void {
  Shapes::idx($shape, 'field_name');
}
