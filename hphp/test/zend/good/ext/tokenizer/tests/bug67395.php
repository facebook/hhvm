<?php

$powToken = token_get_all('<?php **')[1][0];
var_dump(token_name($powToken));

$powEqualToken = token_get_all('<?php **=')[1][0];
var_dump(token_name($powEqualToken));

?>
