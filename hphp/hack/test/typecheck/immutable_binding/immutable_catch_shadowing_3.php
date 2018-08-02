<?hh // experimental

function expect_exception(Exception $e): void {}
function expect_string(string $s): void {}

function func(): void {}

function foo(): void {
  let e = "E";
  try {
    func();
  } catch (Exception f) {
    expect_string(e);
    expect_exception(f);
  }
}
