<?hh

function getMimeType($file) {
  $buffer = file_get_contents($file);
  $finfo = new finfo(FILEINFO_MIME_TYPE);
  return $finfo->buffer($buffer);
}

<<__EntryPoint>>
function main() {
  echo getMimeType(__DIR__ . "/finfo-mime-oom.input") . "\n";
}
