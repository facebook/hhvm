<?php
$res = crypt(b'a', b'_');
if ($res === b'*0' || $res === b'*1') echo 'OK';
else echo 'Not OK';

?>