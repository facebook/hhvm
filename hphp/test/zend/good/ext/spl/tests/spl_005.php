<?php

var_dump(spl_object_hash(new stdClass));
try { var_dump(spl_object_hash(42)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(spl_object_hash()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

?>
===DONE===
<?php exit(0); ?>
