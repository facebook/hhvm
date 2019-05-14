<?php <<__EntryPoint>> function main() {
$simple = simplexml_load_file(dirname(__FILE__)."/book.xml");

var_dump($simple);
echo "Done";
}
