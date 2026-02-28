<?hh


<<__EntryPoint>>
function main_opendir_glob() :mixed{
$dir = opendir('glob://' . __DIR__ . '/../sample_dir/*');

while ( ($file = readdir($dir)) !== false ) {
  echo "$file\n";
}
}
