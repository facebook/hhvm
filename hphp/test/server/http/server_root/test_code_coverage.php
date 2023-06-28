<?hh

function foo() :mixed{
  echo "I'm FOOO\n";
}

<<__EntryPoint>>
function main_test_code_coverage() :mixed{
  if (isset($_GET['enable_code_coverage'])) {
    echo "Code coverage: ".$_GET['enable_code_coverage']."\n";
  }

  try {
    fb_enable_code_coverage();
    foo();
    var_dump(fb_disable_code_coverage());
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
