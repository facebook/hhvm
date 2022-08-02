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

function run_json_on_file_example_3(bool $x): void {
  if ($x && true) {
    return;
  }
  //should provide lint but no quickfix
}
