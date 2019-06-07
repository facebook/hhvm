<?hh


<<__EntryPoint>>
function main_mkdir_recursive_file_exists() {
$dirName = "mkdirRecursiveFileExists";
@unlink($dirName);
fopen($dirName, "w");
var_dump(@mkdir($dirName, 777, true));
unlink($dirName);
}
