<?php
parse_str("123[]=SEGV", $_GET);


var_dump($_REQUEST);
echo "Done\n";

?>