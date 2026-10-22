<?hh

<<__EntryPoint>>
function main(): void {
  echo "Reentrant warning handler\n";
  mb_ereg_search_init('a', '(?<x>a)', 'r');
  set_error_handler(($errno, $message, ...$_) ==> {
    echo "compile warning\n";
    var_dump(mb_ereg_search());
    return true;
  });
  var_dump(mb_ereg_search_init('a', '(?<x>a)', 'd'));
  restore_error_handler();

  echo "Throwing warning handler\n";
  mb_ereg_search_init('a', '(?<x>a)', 'r');
  set_error_handler(($errno, $message, ...$_) ==> {
    throw new Exception('compile warning');
  });
  try {
    mb_ereg_search_init('a', '(?<x>a)', 'd');
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  } finally {
    restore_error_handler();
  }
  var_dump(mb_ereg_search());

  echo "Suppressed warning\n";
  error_reporting(0);
  var_dump(mb_ereg_search_init('a', '(?<x>a)', 'd'));
  var_dump(mb_ereg_search_init('a', '(?<x>a)', 'r'));
  var_dump(mb_ereg_search());

  echo "Independent regex options\n";
  mb_ereg_search_init('A', 'a', 'r');
  var_dump(mb_ereg_match('a', 'A', 'ri'));
  var_dump(mb_ereg_search());

  echo "Independent regex encodings\n";
  mb_regex_encoding('UTF-8');
  mb_ereg_search_init('a', 'a', 'r');
  mb_regex_encoding('ASCII');
  var_dump(mb_ereg_match('a', 'a', 'r'));
  var_dump(mb_ereg_search());
}
