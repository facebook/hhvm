<?php
try
{
	$c = new MongoCollection();
    if ($c) {
        $c->insert(array('hello' => 'Hello, world!', 0 => 'Joehoe'));
    }
}
catch (Exception $e)
{
	echo $e->getMessage();
}
?>