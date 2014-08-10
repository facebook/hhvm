<?php

$streams = array(
	"data:;base64,\0Zm9vYmFyIGZvb2Jhcg==",
	"data:;base64,Zm9vYmFy\0IGZvb2Jhcg==",
	'data:;base64,#Zm9vYmFyIGZvb2Jhcg==',
	'data:;base64,#Zm9vYmFyIGZvb2Jhc=',
	);

foreach($streams as $stream)
{
	var_dump(file_get_contents($stream));
}

?>
===DONE===
<?php exit(0); ?>
