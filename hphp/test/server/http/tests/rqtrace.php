<?hh

<<__EntryPoint>>
function main() :mixed{
  require_once('test_base.inc');
  init();
  requestAll(vec['test_rqtrace.php']);
}
