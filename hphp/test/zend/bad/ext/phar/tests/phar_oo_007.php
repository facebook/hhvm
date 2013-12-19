<?php

require_once 'files/phar_oo_test.inc';

class MyFile extends SplFileObject
{
	function __construct($name)
	{
		echo __METHOD__ . "(" . str_replace(str_replace('\\', '/', dirname(__FILE__)), '*', $name) . ")\n";
		parent::__construct($name);
	}
}

$phar = new Phar($fname);
$phar->setInfoClass('MyFile');

$f = $phar['a.php'];

$s = $f->fstat();

var_dump($s['atime']);
var_dump($s['ctime']);
var_dump($s['mtime']);

var_dump($f->ftell());
var_dump($f->eof());
var_dump($f->fgets());
var_dump($f->eof());
var_dump($f->fseek(20));
var_dump($f->ftell());
var_dump($f->fgets());
var_dump($f->rewind());
var_dump($f->ftell());
var_dump($f->fgets());
var_dump($f->ftell());

?>
===AGAIN===
<?php

$f = $phar['a.php'];

var_dump($f->ftell());
var_dump($f->eof());
var_dump($f->fgets());
var_dump($f->eof());

//unset($f); without unset we check for working refcounting

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>