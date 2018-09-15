<?hh // experimental

function expect_string(string $str): void {}
function expect_int(int $i): void {}

function foo(int $x): void {
  let shadow : string = "Shadow";
  do {
    expect_string(shadow);
    let shadow : int = 42;
    expect_int(shadow);
    $x++;
  } while ($x < 10);
  expect_string(shadow);
}
