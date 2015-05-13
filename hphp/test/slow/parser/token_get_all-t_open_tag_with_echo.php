<?php

$code = '<?=""?>';
$tokens = token_get_all($code);
$firstToken = $tokens[0];
echo token_name($firstToken[0]);
