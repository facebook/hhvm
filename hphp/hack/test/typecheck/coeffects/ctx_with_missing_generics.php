<?hh

function missing_type_params_bad()[cipp_of]: void {}

class Nested<T> {}

function missing_type_params_nested_bad()[cipp_of<\Nested>]: void {}
