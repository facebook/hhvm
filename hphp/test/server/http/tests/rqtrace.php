<?hh

<<__EntryPoint>>
function main() {
  require_once('test_base.inc');
  init();
  requestAll(varray['test_rqtrace.php']);
}
