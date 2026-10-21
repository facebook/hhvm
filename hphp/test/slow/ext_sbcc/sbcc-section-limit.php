<?hh

<<__EntryPoint>>
function main(): void {
  $root = getenv('HPHP_TEST_SBCC_RUN_ROOT');
  require_once($root.'/flib/Greeter.php');
  $global = HH\SBCC\get_global_stats();
  echo "init_errors:".$global['init_errors']."\n";
  echo "value:".SbccLimitGreeter::hello()."\n";
}
