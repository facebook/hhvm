<?hh
// Crafted CDF with a CDF_SIGNED64 property at the end of the property buffer.

<<__EntryPoint>>
function main(): mixed {
  $finfo = new finfo(FILEINFO_MIME_TYPE);
  echo $finfo->file(__DIR__ . '/cdf-property.input') . "\n";
}
