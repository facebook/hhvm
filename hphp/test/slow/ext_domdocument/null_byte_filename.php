<?hh


<<__EntryPoint>>
function main_null_byte_filename() :mixed{
$doc = new DOMDocument();
var_dump($doc->load('/etc/fonts/fonts.conf' . chr(0) . 'somethingelse.xml'));
var_dump($doc->loadHTMLFile(
    '/etc/fonts/fonts.conf' . chr(0) . 'somethingelse.xml'
));
}
