<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_shape_assignment_capture(
  shape('id' => int, 'label' => string) $item,
): void {
  shape('id' => $id, 'label' => $label) = $item;
  $capture = (): (int, string) ==> tuple($id, $label);
}

function test_tuple_assignment_capture((int, string) $item): void {
  tuple($id, $label) = $item;
  $capture = (): (int, string) ==> tuple($id, $label);
}

<<__NoAutoDynamic>>
function test_foreach_shape_key_capture(
  KeyedIterator<shape('id' => int, 'label' => string), (int, string)> $items,
): void {
  foreach ($items as shape('id' => $id, 'label' => $label) => tuple($num, $name)) {
    $capture_key = (): (int, string) ==> tuple($id, $label);
    $capture_value = (): (int, string) ==> tuple($num, $name);
  }
}

<<__NoAutoDynamic>>
function test_foreach_tuple_key_capture(
  KeyedIterator<(int, string), shape('ok' => bool)> $items,
): void {
  foreach ($items as tuple($id, $label) => shape('ok' => $ok)) {
    $capture_key = (): (int, string) ==> tuple($id, $label);
    $capture_value = (): bool ==> $ok;
  }
}
