<?php

$xml = simplexml_load_string(
'<code>
	<a href="javascript:alert(\'1\');"><strong>Item Two</strong></a>
</code>'
);
	
foreach ($xml->xpath("//*") as $element) {
	var_dump($element->asXML());
}

echo "Done\n";
?>