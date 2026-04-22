<?hh

function test_method_call_on_dynamic(dynamic $x): void {
  $x->foo();
}

function test_function_call_on_dynamic(dynamic $x): void {
  ($x)();
}

function test_property_access_on_dynamic(dynamic $x): void {
  $y = $x->bar;
}

function test_method_call_through_intersection(dynamic $x): void {
  if ($x is C) {
    $x->bar();
  }
}

function test_function_call_through_intersection(dynamic $x): void {
  if ($x is C) {
    ($x)();
  }
}

class C {}

function test_no_warn_on_non_dynamic(int $x): void {
  // No warning expected
}

function test_array_index_on_dynamic(dynamic $x): void {
  $y = $x[0];
}

function test_array_update_on_dynamic(dynamic $x): void {
  $x[0] = 42;
}

function test_array_append_on_dynamic(dynamic $x): void {
  $x[] = 42;
}

function test_array_index_through_intersection(dynamic $x): void {
  if ($x is C) {
    $y = $x[0];
  }
}

function test_property_access_through_intersection(dynamic $x): void {
  if ($x is C) {
    $y = $x->bar;
  }
}

function test_no_array_warn_on_non_dynamic(vec<int> $x): void {
  $y = $x[0];
}
