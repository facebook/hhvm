<?php


<<__EntryPoint>>
function main_token_get_all_hashbang() {
$code = <<<'EOC'
#!/usr/bin/env php
another line
<?php $x ?>
EOC;
var_dump(token_get_all($code));
}
