<?hh

require_once('test_base.inc');
<<__EntryPoint>> function main(): void {
echo "Enable Code Coverage = 0\n";

requestAll(
  varray[
    "test_code_coverage.php",
  ],
  "-vEval.EnableCodeCoverage=0"
);

echo "Enable Code Coverage = 1\n";

requestAll(
  varray[
    "test_code_coverage.php",
    "test_code_coverage.php?enable_code_coverage=false",
    "test_code_coverage.php?enable_code_coverage=true",
  ],
  "-vEval.EnableCodeCoverage=1"
);

echo "Enable Code Coverage = 2\n";

requestAll(
  varray[
    "test_code_coverage.php",
  ],
  "-vEval.EnableCodeCoverage=2"
);
}
