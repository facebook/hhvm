<?hh // strict

function test_lambda_sighelp(string $str, (function(string): int) $f): int {
  return $f($str);
}
function normal_test_func(string $str): void {}

function use_lambda_sighelp(): int {
  return test_lambda_sighelp("Hello", $str ==> {
    normal_test_func("hi");
    return 2 * (1 - 3);
  });
}
