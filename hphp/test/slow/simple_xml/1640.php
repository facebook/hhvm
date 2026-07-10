<?hh


<<__EntryPoint>>
function main_1640() :mixed{
$sxe = new SimpleXMLElement('<image-definition />');
$sxe->addChild('path', 'some/path/to/my.file');
$sxe->addChild('options');
$sxe->options->addChild('paddingbottom', '1');
var_dump($sxe->path->__toString());
var_dump($sxe->options->paddingbottom->__toString());
}
