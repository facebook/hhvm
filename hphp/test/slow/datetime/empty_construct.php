<?hh


<<__EntryPoint>>
function main_empty_construct() {
$a = new DateTime('', new DateTimeZone('Pacific/Tahiti'));
$a->setTimestamp(1234567890);
var_dump($a->format('c'));
}
