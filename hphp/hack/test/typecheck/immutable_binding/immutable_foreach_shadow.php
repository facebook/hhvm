<?hh // experimental

function expect_int(int $i): void {}
function expect_string(string $i): void {}

function it(array<int> $arr): void {
  foreach ($arr as element) {
    expect_int(element);
    let element = (string)element;
    expect_string(element);
  }
}
