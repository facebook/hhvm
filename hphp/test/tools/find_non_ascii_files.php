<?hh
/**
 * When importing Zend tests there are a lot of them. Sometime differential does
 * not like some of the files that are included. So, if that is the case, use
 * this script to mark any files as binary that are not ASCII. It makes life
 * easier for differential and phabricator.

 * e.g., Use this after importing Zend tests and you run into an exception
 * during the differential process like:
 *
 * EXCEPTION: (ConduitClientException) ERR-CONDUIT-CORE: Attempting to construct
 * a query containing characters outside of the Unicode Basic Multilingual
 * Plane. MySQL will silently truncate this data if it is inserted into a
 * `utf8` column. Use the `%B` conversion to escape binary strings data.
 *
 **/

function find_bad_encoding_files(string $start_dir): Set<string> {
  $bad_files = Set {};
  $rdit = new RecursiveDirectoryIterator(__DIR__ . "/../zend/");
  foreach (new RecursiveIteratorIterator($rdit) as $file) {
    $contents = file_get_contents($file);
    $encoding = mb_detect_encoding($contents);
    if ($encoding !== "ASCII") {
      $bad_files[] = $file->__toString();
    }
  }
  return $bad_files;
}

function add_to_git_attributes(string $git_attr_file, string $filename): void {
  $data = $filename . " binary" . PHP_EOL;
  $current_attr_file_contents = "";
  if (file_exists($git_attr_file)) {
    $current_attr_file_contents = file_get_contents($git_attr_file);
  }
  // Don't add to .gitattributes file if it already exists in there
  if (strpos($current_attr_file_contents, $data) === false) {
    file_put_contents($git_attr_file, $data, FILE_APPEND);
  }
}

function main(): void {
  $zend_dir = __DIR__ . "../zend/";
  $bad_files = find_bad_encoding_files($zend_dir);
  $git_attr_file = __DIR__ . "/../zend/.gitattributes";
  foreach ($bad_files as $filename) {
    // Remove the part of the filename up to and including "test/zend/"
    $filename = substr_replace($filename, "", 0, strlen($zend_dir) + 1);
    add_to_git_attributes($git_attr_file, $filename);
  }
}

main();
