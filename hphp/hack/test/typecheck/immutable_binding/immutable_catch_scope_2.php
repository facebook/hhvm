<?hh // experimental

function expect_exception(Exception $e): void {}
function expect_string(string $s): void {}

function func(): void {}

function foo(): void {
  try {
    func();
  } catch (Exception e) {
    expect_exception(e);
  }
  echo e;
}
