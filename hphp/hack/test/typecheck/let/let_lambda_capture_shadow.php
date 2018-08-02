<?hh // experimental

function expect_int(int $i): void {}
function expect_string(string $s): void {}

function foo(): void {
  let count = 10;
  let repeat = () ==> {
    for ($i = 0; $i < count; $i++) {
      expect_int(count);
      let count = "count";
      expect_string(count);
    }
  };
  expect_int(count);
  repeat();
}
