<?hh


<<__EntryPoint>>
function main_directory_iterator_ctor_emtpystring() :mixed{
try {
  new DirectoryIterator("");
} catch (RuntimeException $e) {
  echo "RuntimeException: " . $e->getMessage() . "\n";
}
}
