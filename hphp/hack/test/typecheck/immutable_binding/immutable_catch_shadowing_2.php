<?hh // experimental

class ExA extends Exception {}
class ExB extends Exception {}

function expect_ex_a(ExA $e): void {}
function expect_exception(Exception $e): void {}
function expect_string(string $s): void {}

function func(): void {}

function foo(): void {
  try {
    func();
  } catch (ExA e) {
    expect_ex_a(e);
    expect_exception(e);
    let e = "MyE";
    expect_string(e);
  } catch (ExB e) {
    expect_ex_a(e); // error
  }
}
