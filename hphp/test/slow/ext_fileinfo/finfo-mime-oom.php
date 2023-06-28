<?hh

function getMimeType($file) :mixed{
  $buffer = file_get_contents($file);
  $finfo = new finfo(FILEINFO_MIME_TYPE);
  return $finfo->buffer($buffer);
}

<<__EntryPoint>>
function main() :mixed{
  echo getMimeType(__DIR__ . "/finfo-mime-oom.input") . "\n";
}
