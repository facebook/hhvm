<?hh


<<__EntryPoint>>
function main_tar_archive() {
$handler = new \__SystemLib\TarArchiveHandler(__DIR__ . '/tar_archive.tar');
var_dump($handler->getEntriesMap()->keys());
}
