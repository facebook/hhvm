<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function takes_representable_as_int(\HH\Runtime\RepresentableAs<int> $c): void {}
function takes_representable_as_num(\HH\Runtime\RepresentableAs<num> $c): void {}

function test_t_sub_representable_as_t(int $i): void {
  // int <: RepresentableAs<int> — should pass
  takes_representable_as_int($i);
}

function test_int_sub_representable_as_num(int $i): void {
  // int <: RepresentableAs<num> — should pass (int <: num)
  takes_representable_as_num($i);
}

function test_num_sub_representable_as_int_fails(num $n): void {
  // num <: RepresentableAs<int> — should FAIL
  takes_representable_as_int($n);
}

function test_string_sub_representable_as_int_fails(string $s): void {
  // string <: RepresentableAs<int> — should FAIL
  takes_representable_as_int($s);
}
