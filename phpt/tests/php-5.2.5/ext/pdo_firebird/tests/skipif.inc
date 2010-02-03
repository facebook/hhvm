<?php /* $Id: skipif.inc,v 1.1 2004/06/11 01:37:06 abies Exp $ */

if (!extension_loaded("interbase") || !extension_loaded("pdo_firebird")) print "skip"; 

?>
