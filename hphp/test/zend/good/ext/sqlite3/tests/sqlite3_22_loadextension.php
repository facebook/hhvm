<?hh

require_once(dirname(__FILE__) . '/new_db.inc');

var_dump($db->loadExtension('myext.txt'));
var_dump($db->close());

echo "Done\n";
