<?php
class A extends PDO
{ function __call($m, $p) {print __CLASS__."::$m\n";} }

$a = new A('sqlite:' . __DIR__ . '/dummy.db');

$a->truc();
$a->TRUC();

?>
<?php error_reporting(0); ?>
<?php
unlink(__DIR__ . '/dummy.db');
?>