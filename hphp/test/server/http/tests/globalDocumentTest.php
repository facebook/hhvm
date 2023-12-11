<?hh

<<__EntryPoint>>
function main() :mixed{
require_once('test_base.inc');
init();
require_once('globalDocumentTestUtils.inc');

echo "No global doc\n";

requestAll(
  vec[
    "global_document_should_not_be_hit.php",
  ],
  "",
);

echo "Global doc, existing file\n";

requestAll(
  vec[
    "global_document_should_not_be_hit.php",
  ],
  "-vServer.GlobalDocument=/global_document.php",
);

echo "Global doc, nonexistent file\n";

requestAll(
  vec[
    "this_file_does_not_exist.php",
    "foo/this_file_does_not_exist.php",
    "foo/bar/this_file_does_not_exist.php",
  ],
  "-vServer.GlobalDocument=/global_document.php",
);

echo "Nonexistent global doc\n";

runTestWith404HealthCheck(
  function($serverPort) {
    $nonexistent_requests = vec[
      "some_file.php",
      "this_file_does_not_exist.php",
      "index.php",
      "directory_that_exists",
    ];

    foreach ($nonexistent_requests as $request) {
      echo "Requesting '$request'\n";
      var_dump(request('localhost', $serverPort, $request));
    }
  },
  "-vServer.GlobalDocument=/this_file_does_not_exist.php",
);
}
