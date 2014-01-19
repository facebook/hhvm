<?php 
chdir(dirname(__FILE__));
$fp=fopen("004.data","r");
while($a=fgetcsv($fp,100,"\t")) {
	echo join(",",$a)."\n";
}
fclose($fp);
?>