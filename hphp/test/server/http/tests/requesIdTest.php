<?hh

<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
requestAll(varray[
  "test_get_request_id.php",
  "test_get_request_id.php",
  "test_get_request_id.php",
], '-vServer.ThreadCount=1');
}

