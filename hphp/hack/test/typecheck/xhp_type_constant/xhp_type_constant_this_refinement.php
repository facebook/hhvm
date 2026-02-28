<?hh
<<file:__EnableUnstableFeatures('xhp_type_constants')>>

abstract class :base extends XHPTest {
  abstract const type TData as shape(?'id' => ?int, ...) =
    shape(?'id' => ?int, ...);

  attribute this::TData data @required;

  public function isIdDefined(): bool {
    return Shapes::idx($this->:data, 'id') !== null;
  }
}

final class :child extends :base {
  const type TData = shape('id' => int, 'name' => string);
}

function test_refined_data(:child $obj): shape('id' => int, 'name' => string) {
  return $obj->:data;
}

function test_base_data(:base $obj): shape(?'id' => ?int, ...) {
  return $obj->:data;
}

function create_child(): :child {
  return <child data={shape('id' => 123, 'name' => 'test')} />;
}

function test_is_id_defined(): bool {
  $child = create_child();
  return $child->isIdDefined();
}
