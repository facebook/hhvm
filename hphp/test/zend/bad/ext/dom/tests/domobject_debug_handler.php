<?php
$xml = <<<XML
<foo>
	<bar>foobar</bar>
</foo>
XML;
$d = new domdocument;
$d->dynamicProperty = new stdclass;
$d->loadXML($xml);
print_r($d);
