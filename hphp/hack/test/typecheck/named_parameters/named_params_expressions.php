<?hh

function expr_test(int $pos, named string $label): void {}

function get_string(): string {
  return "dynamic";
}

function test_expressions(): void {
  expr_test(label="pre" . "fix", 42);

  expr_test(label=get_string(), 100);

  $x = 42;
  $s = "test";
  expr_test(label=$s . "suffix", $x);

  $strings = vec["hello", "world"];
  expr_test(label=$strings[0] . " " . $strings[1], 999);
}
