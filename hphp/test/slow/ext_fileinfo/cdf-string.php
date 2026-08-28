<?hh
// Crafted CDF_LENGTH32_STRING whose s_len exceeds the stream buffer.

<<__EntryPoint>>
function main(): mixed {
  $finfo = new finfo(FILEINFO_MIME_TYPE);
  echo $finfo->file(__DIR__ . '/cdf-string.input') . "\n";
}
