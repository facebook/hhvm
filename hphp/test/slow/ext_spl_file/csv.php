<?hh


<<__EntryPoint>>
function main_csv() :mixed{
$file = new SplFileObject(__DIR__ . '/csv.csv');
$file->setFlags(SplFileObject::READ_CSV);
$file->setCsvControl(';');
var_dump($file->getCsvControl());

foreach ($file as $line) {
  var_dump($line);
  exit;
}
}
