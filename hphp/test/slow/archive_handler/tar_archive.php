<?hh

$handler = new \__SystemLib\TarArchiveHandler(__DIR__ . '/tar_archive.tar');
var_dump($handler->getContentsList()->keys());
