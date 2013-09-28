<?php

var_dump(pathinfo());
var_dump(pathinfo(""));
var_dump(pathinfo("."));
var_dump(pathinfo(".."));
var_dump(pathinfo("/"));
var_dump(pathinfo("./"));
var_dump(pathinfo("/."));
var_dump(pathinfo(".cvsignore"));
var_dump(pathinfo(__FILE__, PATHINFO_BASENAME));
var_dump(pathinfo(__FILE__, PATHINFO_FILENAME));
var_dump(pathinfo(__FILE__, PATHINFO_EXTENSION));
var_dump(pathinfo(__FILE__, PATHINFO_DIRNAME));
var_dump(pathinfo(__FILE__, PATHINFO_EXTENSION|PATHINFO_FILENAME|PATHINFO_DIRNAME));
var_dump(pathinfo(__FILE__, PATHINFO_EXTENSION|PATHINFO_FILENAME|PATHINFO_BASENAME));
var_dump(pathinfo(__FILE__, PATHINFO_EXTENSION|PATHINFO_FILENAME));
var_dump(pathinfo(__FILE__, PATHINFO_EXTENSION|PATHINFO_BASENAME));
var_dump(pathinfo(__FILE__, PATHINFO_FILENAME|PATHINFO_DIRNAME));
var_dump(pathinfo(__FILE__, PATHINFO_FILENAME|PATHINFO_BASENAME));
var_dump(pathinfo(__FILE__, PATHINFO_DIRNAME|PATHINFO_EXTENSION));
var_dump(pathinfo(__FILE__, PATHINFO_DIRNAME|PATHINFO_BASENAME));

echo "Done\n";
?>