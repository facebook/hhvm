<?hh
// @format

function run_json_on_file(bool $x): void {
  if ($x == true) {
    return;
  } //should provide quickfixes
}

function run_json_on_file_example_2(bool $x): void {
  if (true == $x) {
    return;
  } //should provide quickfixes
}

function run_json_on_file_example_1_false(bool $x): void {
  if ($x === false) {
    return;
  } //should provide quickfixes
}

function run_json_on_file_example_2_false(bool $x): void {
  if (false === $x) {
    return;
  } //should provide quickfixes
}
