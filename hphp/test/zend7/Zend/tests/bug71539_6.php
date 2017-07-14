<?php
$name = 'a';
for ($i = 0; $i < 100000; $i++) {
    if ($name != 'i') {
        $$name =& $GLOBALS;
    }
    $name++;
}
?>
OK
