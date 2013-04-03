<?php

$x = new Exception;
$x->gettraceasstring(1);
$x->gettraceasstring();
$x->__tostring(1);
$x->gettrace(1);
$x->getline(1);
$x->getfile(1);
$x->getmessage(1);
$x->getcode(1);

?>