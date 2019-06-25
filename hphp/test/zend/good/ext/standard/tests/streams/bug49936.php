<?hh
<<__EntryPoint>> function main(): void {
$dir = 'ftp://your:self@localhost/';

var_dump(@opendir($dir));
var_dump(@opendir($dir));

echo "===DONE===\n";
}
