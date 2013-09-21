<?php

chdir("/");
var_dump(mkdir("./testdir/subdir", 0777, true));
var_dump(rmdir("./testdir/subdir"));
var_dump(rmdir("./testdir"));

echo "Done\n";
?>