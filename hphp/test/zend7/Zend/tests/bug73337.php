<?php
class d { function __destruct() { throw new Exception; } }
try { new d + new d; } catch (Exception $e) { print "Exception properly caught\n"; }
?>
