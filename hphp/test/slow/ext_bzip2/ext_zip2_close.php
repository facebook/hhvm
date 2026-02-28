<?hh


<<__EntryPoint>>
function main_ext_zip2_close() :mixed{
ini_set( 'display_errors', 'stderr' );
$reader = new XMLReader();
$url = "compress.bzip2://".dirname(__FILE__)."/book.xml.bz2";
var_dump($reader->open( $url, null ));
var_dump($reader->read());
}
