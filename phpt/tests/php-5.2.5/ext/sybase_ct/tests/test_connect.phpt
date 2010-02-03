--TEST--
Sybase-CT connectivity
--SKIPIF--
<?php require('skipif.inc'); ?>
--FILE--
<?php
/* This file is part of PHP test framework for ext/sybase_ct
 *
 * $Id: test_connect.phpt,v 1.1 2004/01/24 02:18:13 thekid Exp $ 
 */

  require('test.inc');

  $db= sybase_connect_ex();
  var_dump($db);
  sybase_close($db);
?>
--EXPECTF--
resource(%d) of type (sybase-ct link)
