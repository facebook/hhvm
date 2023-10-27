<?hh


function foo_per_file() :mixed{
  echo "I'm FOOO\n";
}


<<__EntryPoint>>
function main_test_per_file_coverage() :mixed{

  require_once('test_per_file_coverage.inc');

  if (isset($_GET['enable_per_file_coverage'])) {
    echo "Per file coverage: ".$_GET['enable_per_file_coverage']."\n";
  }

  try {
    HH\enable_per_file_coverage(keyset[__FILE__, __DIR__."/test_per_file_coverage.inc", __DIR__."/test_status.php"]);
    foo_per_file();
    var_dump(17 is ITest);
    (new CTest())->foo();
    var_dump(HH\get_all_coverage_data());
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
