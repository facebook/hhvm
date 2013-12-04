<?php
ini_set('allow_url_fopen', 1);


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
<?php
ini_set('allow_url_fopen', 1);
 exit(0); ?>