<?hh


<<__EntryPoint>>
function main_1653() :mixed{
$sxe = new SimpleXMLElement('<foo />');
$sxe->addChild('options');
$sxe->options->addChild('paddingtop', '0');
echo 'Success
';
}
