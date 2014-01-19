<?php
$obj = New SplFileObject(__FILE__);
$obj->fseek(1,2,3);
$obj->fseek();
?>