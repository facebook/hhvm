<?php


<<__EntryPoint>>
function main_token_get_all_t_open_tag_with_echo() {
$code = '<?=""?>';
$tokens = token_get_all($code);
$firstToken = $tokens[0];
echo token_name($firstToken[0]);
}
