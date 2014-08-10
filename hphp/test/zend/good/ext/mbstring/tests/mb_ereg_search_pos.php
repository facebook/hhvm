<?php
mb_regex_encoding('iso-8859-1');
$test_str = 'Iñtërnâtiônàlizætiøn';

if(mb_ereg_search_init($test_str))
{
	$val=mb_ereg_search_pos("nâtiôn");
	
	var_dump($val);
	
}
else{
	var_dump("false");
}
?>
