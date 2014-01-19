<?php

$string = <<<EOT
Cooking_furniture="KÃ¼chen MÃ¶bel (en)"
KÃ¼chen_MÃ¶bel="Cooking furniture (en)"
EOT;

$filename = dirname(__FILE__) . '/bug49056.tmp';

file_put_contents( $filename, $string);

var_dump(parse_ini_file($filename));

?>
<?php @unlink(dirname(__FILE__) . '/bug49056.tmp'); ?>