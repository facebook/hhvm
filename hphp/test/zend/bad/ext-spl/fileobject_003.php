<?php

function test($name, $lc, $lp)
{
	static $i = 0;
	echo "===$i===\n";
	$i++;

	$o = new SplFileInfo($name);

	var_dump($o);
	$c = clone $o;
	var_dump($c);
	var_dump($o === $c);
	var_dump($o == $c);
	var_dump($o->getPathname() == $c->getPathname());
	
	try {
		$f = new SplFileObject($name);
		var_dump($name);
		var_dump($f->getPathName());
		$l = substr($f->getPathName(), -1);
		var_dump($l != '/' && $l != '\\' && $l == $lc);
		var_dump($f->getFileName());
		$l = substr($f->getFileName(), -1);
		var_dump($l != '/' && $l != '\\' && $l == $lc);
		var_dump($f->getPath());
		$l = substr($f->getPath(), -1);
		var_dump($l != '/' && $l != '\\' && $l == $lp);
	} catch (LogicException $e) {
		echo "LogicException: ".$e->getMessage()."\n";
	}
	try {
		$fo = $o->openFile();
		var_dump($fo->getPathName(), $fo->getFileName(), $fo->getPath());
	} catch (LogicException $e) {
		echo "LogicException: ".$e->getMessage()."\n";
	}
}

test(dirname(__FILE__) . '/' . 'fileobject_001a.txt', 't', substr(dirname(__FILE__),-1));
test(dirname(__FILE__) . '/', substr(dirname(__FILE__),-1), 'l');
test(dirname(__FILE__),       substr(dirname(__FILE__),-1), 'l');

?>
===DONE===
<?php exit(0); ?>