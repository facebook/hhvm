<?hh
// Crafted CDF whose directory d_name has no NUL terminator.

<<__EntryPoint>>
function main(): mixed {
  $finfo = new finfo(FILEINFO_MIME_TYPE);
  echo $finfo->file(__DIR__ . '/cdf-dirname.input') . "\n";
}
