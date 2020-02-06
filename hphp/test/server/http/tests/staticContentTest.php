<?hh

<<__EntryPoint>> function main(): void {
require_once('test_base.inc');

// Limit the server to 1 thread only to ensure that both requests are served by
// the same thread
$customArgs = "-d hhvm.server.thread_count=1";

// Test static content request before the creation of any HPHP sessions
requestAll(
  varray[
    "static_content.txt",
    "test_get.php?name=Foo",
  ],
  $customArgs
);

// Test static content request after a HPHP session has terminated
requestAll(
  varray[
    "test_get.php?name=Foo",
    "static_content.txt",
  ],
  $customArgs
);
}
