<?hh

function get_value_from_ref_via_elvis(readonly HH\Lib\Ref<bool> $ref): ?bool {
  return $ref->value ?: null; // expect readonly error
}

function get_value_from_ref_via_ternary(readonly HH\Lib\Ref<bool> $ref): ?bool {
  return $ref->value ? $ref->value : null; // expect readonly error
}

function use_value_as_condition(readonly HH\Lib\Ref<bool> $ref): ?int {
  return $ref->value ? 1 : null; // expect no error
}
