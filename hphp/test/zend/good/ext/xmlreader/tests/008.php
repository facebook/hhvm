<?hh
/* $Id$ */
<<__EntryPoint>> function main(): void {
$dtdfile = rawurlencode(realpath(__DIR__ . '/dtdexample.dtd'));
$xmlstring = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE LIST SYSTEM "' . $dtdfile . '">
<LIST>
<MOVIE ID="x200338360">
<TITLE>Move Title 1</TITLE>
<ORGTITLE/><LOC>Location 1</LOC>
<INFO/>
</MOVIE>
<MOVIE ID="m200338361">
<TITLE>Move Title 2</TITLE>
<ORGTITLE/>
<LOC>Location 2</LOC>
<INFO/>
</MOVIE>
</LIST>';

$file = sys_get_temp_dir().'/'.'_008.xml';
file_put_contents($file, $xmlstring);


$reader = new XMLReader();
$reader->open($file);
$reader->setParserProperty(XMLReader::LOADDTD, TRUE);
$reader->setParserProperty(XMLReader::VALIDATE, TRUE);
while($reader->read());
if ($reader->isValid()) {
    echo "file DTD: ok\n";
}
$reader->close();
unlink($file);

$dtdfile = rawurlencode(dirname(__FILE__) . '/dtdexample.dtd');
$xmlstring = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE LIST SYSTEM "file:///' . $dtdfile. '">
<LIST>
<MOVIE ID="x200338360">
<TITLE>Move Title 1</TITLE>
<ORGTITLE/><LOC>Location 1</LOC>
<INFO/>
</MOVIE>
<MOVIE ID="m200338361">
<TITLE>Move Title 2</TITLE>
<ORGTITLE/>
<LOC>Location 2</LOC>
<INFO/>
</MOVIE>
</LIST>';

$reader = new XMLReader();
$reader->XML($xmlstring);

$reader->setParserProperty(XMLReader::LOADDTD, TRUE);
$reader->setParserProperty(XMLReader::VALIDATE, TRUE);
while($reader->read());
if ($reader->isValid()) {
    echo "string DTD: ok\n";
}
echo "===DONE===\n";
}
