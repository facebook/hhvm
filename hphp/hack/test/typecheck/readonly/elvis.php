<?hh

function get_value_from_ref_via_elvis<T>(readonly HH\Lib\Ref<T> $ref): ?T {
  return $ref->value ?: null; // expect readonly error
}

function get_value_from_ref_via_ternary<T>(readonly HH\Lib\Ref<T> $ref): ?T {
  return $ref->value ? $ref->value : null; // expect readonly error
}

function use_value_as_condition<T>(readonly HH\Lib\Ref<T> $ref): ?int {
  return $ref->value ? 1 : null; // expect no error
}
