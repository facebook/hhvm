<?php
    var_dump(posix_ttyname()); // param missing
    var_dump(posix_ttyname(0)); // param not a ressource
    var_dump(posix_ttyname(imagecreate(1, 1))); // wrong resource type
?>
===DONE===