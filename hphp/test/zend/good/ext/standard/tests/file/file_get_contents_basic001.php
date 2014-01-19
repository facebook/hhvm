<?php
	$file_content = "Bienvenue au CodeFest a Montreal";
	$temp_filename = dirname(__FILE__)."/fichier_a_lire.txt";
	$handle = fopen($temp_filename,"w");
	fwrite($handle,$file_content);
	fclose($handle);
	$var = file_get_contents($temp_filename);
	echo $var;
?>
<?php
	$temp_filename = dirname(__FILE__)."/fichier_a_lire.txt";
	unlink($temp_filename);
?>