<?php
echo basename("c:file.txt") . "\n";
echo basename("d:subdir\\file.txt") . "\n";
echo basename("y:file.txt", ".txt") . "\n";
echo basename("notdriveletter:file.txt") . "\n";
?>
==DONE==