<?hh

function missing_type_params_bad()[policied_of]: void {}

class Nested<T> {}

function missing_type_params_nested_bad()[policied_of<\Nested>]: void {}
