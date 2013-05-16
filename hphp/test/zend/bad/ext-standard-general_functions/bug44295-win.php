<?php
$dir = 'c:\\not\\exists\\here';

set_error_handler('my_error_handler');
function my_error_handler() {$a = func_get_args(); print "in error handler\n"; }

try {
        print "before\n";
        $iter = new DirectoryIterator($dir);
        print get_class($iter) . "\n";
        print "after\n";
} catch (Exception $e) {
        print "in catch: ".$e->getMessage()."\n";
}
?>
==DONE==
<?php exit(0); ?>