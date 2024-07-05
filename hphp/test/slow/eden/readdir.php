<?hh

<<__EntryPoint>>
function main(): void {
  $files = HH\Eden\list_files_recursive(__DIR__);
  echo "=== All files in directory ===\n";
  var_dump($files);

  echo "=== List a nonexistent directory ===\n";
  try {
    $files = HH\Eden\list_files_recursive(__DIR__.'/nonexistent');
  } catch (Exception $ex) {
    $msg = $ex->getMessage();
    $is_readlink_error = HH\Lib\Str\starts_with($msg, 'Failed to readlink');
    echo "Got a Failed To Readlink error: ". ($is_readlink_error ? 'true' : 'false'). "\n";
  }
  echo "=== Try to establish client in non-Eden mount ===\n";
  try {
    $client = HH\Eden\list_files_recursive("/tmp");
  } catch (Exception $ex) {
    echo "Got an exception: ".$ex->getMessage()."\n";
  }
}
