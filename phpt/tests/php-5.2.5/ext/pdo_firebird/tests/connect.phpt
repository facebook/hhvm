--TEST--
PDO_Firebird: connect/disconnect
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php /* $Id: connect.phpt,v 1.1 2004/06/11 01:37:06 abies Exp $ */

	require("testdb.inc");
    
	$db = new PDO("firebird:dbname=$test_base",$user,$password) or die;
	unset($db);
	echo "done\n";
	
?>
--EXPECT--
done
