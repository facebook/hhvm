<?hh


<<__EntryPoint>>
function main_opendir_glob() :mixed{
$dir = opendir('glob://' . __DIR__ . '/../sample_dir/*');

$file = readdir($dir);
while ( $file !== false ) {
  echo "$file\n";
  $file = readdir($dir);
}
}
