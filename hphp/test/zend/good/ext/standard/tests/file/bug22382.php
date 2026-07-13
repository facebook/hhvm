<?hh <<__EntryPoint>> function main(): void {
$fp = fopen(dirname(__FILE__)."/test2.csv", "r");
$line = fgetcsv($fp, 1024);
while($line) {
    var_dump($line);
    $line = fgetcsv($fp, 1024);
}
fclose($fp);
}
