<?php  

ini_set('session.save_handler', 'files');
$x = new SessionHandler;
$x->open('','');
$x->open('','');
$x->open('','');
$x->open('','');

print "Done!\n";

?>