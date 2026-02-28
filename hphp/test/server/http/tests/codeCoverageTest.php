<?hh

<<__EntryPoint>> function main(): void {
require_once('test_base.inc');
init();
echo "Enable Code Coverage = 0\n";

requestAll(
  vec[
    "test_code_coverage.php",
  ],
  "-vEval.EnableCodeCoverage=0"
);

echo "Enable Code Coverage = 1\n";

requestAll(
  vec[
    "test_code_coverage.php",
    "test_code_coverage.php?enable_code_coverage=false",
    "test_code_coverage.php?enable_code_coverage=true",
  ],
  "-vEval.EnableCodeCoverage=1"
);

echo "Enable Code Coverage = 2\n";

requestAll(
  vec[
    "test_code_coverage.php",
  ],
  "-vEval.EnableCodeCoverage=2"
);

echo "Enable Code Coverage = 1 && Enable Per File Coverage = 1\n";

requestAll(
  vec[
    "test_code_coverage.php",
    "test_code_coverage.php?enable_code_coverage=true",
    "test_code_coverage.php?enable_code_coverage=true&enable_per_file_coverage=true",
    "test_per_file_coverage.php",
    "test_per_file_coverage.php?enable_code_coverage=true",
    "test_per_file_coverage.php?enable_per_file_coverage=true",
  ],
  "-vEval.EnableCodeCoverage=1 -vEval.EnablePerFileCoverage=1"
);

}
