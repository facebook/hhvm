<?php
$crypt = crypt(b'U*U', b'$2a$CCCCCCCCCCCCCCCCCCCCC.E5YPO9kmyuRGyh0XouQYb4YMJKvyOeW');
if ($crypt===b'$2SHYF.wPGyfE') {
    echo "OK\n";
} else {
    echo "Not OK\n";
}
?>