<?hh


<<__EntryPoint>>
function main_persistent() :mixed{
$dsn = 'sqlite:'.realpath(__DIR__).'/persistent.db';
$pdo = new PDO($dsn, '', '', darray[PDO::ATTR_PERSISTENT => true]);
var_dump('success');
}
