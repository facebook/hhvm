<?hh <<__EntryPoint>> function main(): void {
$fp = fopen(dirname(__FILE__)."/test.csv", "r");
$line = fgetcsv($fp, 24);
while($line) {
    $line = str_replace("\x0d\x0a", "\x0a", $line);
    var_dump($line);
    $line = fgetcsv($fp, 24);
}
fclose($fp);
}
