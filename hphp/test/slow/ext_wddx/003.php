<?php
	$path = dirname(__FILE__);
	$fp = fopen("php://temp", 'w+');
	fputs($fp, "<wddxPacket version='1.0'><header><comment>TEST comment</comment></header><data><struct><var name='var1'><null/></var><var name='var2'><string>some string</string></var><var name='var3'><number>756</number></var><var name='var4'><boolean value='true'/></var></struct></data></wddxPacket>");
	rewind($fp);
	var_dump(wddx_deserialize($fp));
	fclose($fp);
?>
