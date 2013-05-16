<?php 
$xml =<<<EOF
<people>
  <person name="Joe"/>
  <person name="John">
    <children>
      <person name="Joe"/>
    </children>
  </person>
  <person name="Jane"/>
</people>
EOF;

$people = simplexml_load_string($xml);

foreach($people as $person)
{
	var_dump((string)$person['name']);
	var_dump(count($people));
	var_dump(count($person));
}

?>
===DONE===