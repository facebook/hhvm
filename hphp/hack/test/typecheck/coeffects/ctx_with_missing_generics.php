<?hh

function missing_type_params_bad()[zoned_with]: void {}

class Nested<T> {}

function missing_type_params_nested_bad()[zoned_with<\Nested>]: void {}
