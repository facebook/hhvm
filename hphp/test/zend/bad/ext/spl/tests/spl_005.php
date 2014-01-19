<?php

var_dump(spl_object_hash(new stdClass));
var_dump(spl_object_hash(42));
var_dump(spl_object_hash());

?>
===DONE===
<?php exit(0); ?>