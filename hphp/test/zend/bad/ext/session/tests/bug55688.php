<?php
ini_set('session.save_handler', 'files');
$x = new SessionHandler;
$x->gc(1);
?>