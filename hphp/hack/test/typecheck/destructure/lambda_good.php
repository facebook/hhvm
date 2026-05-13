<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_shape_assignment_capture(
  shape('id' => int, 'label' => string) $item,
): void {
  shape('id' => $id, 'label' => $label) = $item;
  $capture = (): (int, string) ==> tuple($id, $label);
  hh_expect_equivalent<(function(): (int, string))>($capture);
}

function test_tuple_assignment_capture((int, string) $item): void {
  tuple($id, $label) = $item;
  $capture = (): (int, string) ==> tuple($id, $label);
  hh_expect_equivalent<(function(): (int, string))>($capture);
}

function test_foreach_shape_key_capture(
  KeyedIterator<shape('id' => int, 'label' => string), (int, string)> $items,
): void {
  foreach ($items as shape('id' => $id, 'label' => $label) => tuple($num, $name)) {
    $capture_key = (): (int, string) ==> tuple($id, $label);
    $capture_value = (): (int, string) ==> tuple($num, $name);
    hh_expect_equivalent<(function(): (int, string))>($capture_key);
    hh_expect_equivalent<(function(): (int, string))>($capture_value);
  }
}

function test_foreach_tuple_key_capture(
  KeyedIterator<(int, string), shape('ok' => bool)> $items,
): void {
  foreach ($items as tuple($id, $label) => shape('ok' => $ok)) {
    $capture_key = (): (int, string) ==> tuple($id, $label);
    $capture_value = (): bool ==> $ok;
    hh_expect_equivalent<(function(): (int, string))>($capture_key);
    hh_expect_equivalent<(function(): bool)>($capture_value);
  }
}
