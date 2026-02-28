<?hh <<__EntryPoint>> function main(): void {
$f = dirname(__FILE__)  . '/bug49072.zip';
$o = new ZipArchive();
if (! $o->open($f, ZipArchive::CHECKCONS)) {
    exit ('error can\'t open');
}
$r = $o->getStream('file1'); // this file has a wrong crc
if (!$r)exit('failed to open a stream for file1');
$s = '';
while (! feof($r)) {
    $s .= fread($r,1024);
}
}
