<?hh // experimental

class MyE extends Exception {}

function expect_my_e(MyE $e): void {}
function expect_exception(Exception $e): void {}
function expect_string(string $s): void {}

function func(): void {}

function foo(): void {
  try {
    func();
  } catch (MyE e) {
    expect_my_e(e);
    expect_exception(e);
    let e = "MyE";
    expect_string(e);
  } catch (Exception e) {
    expect_exception(e);
    let e = "Exception";
    expect_string(e);
  }
}
