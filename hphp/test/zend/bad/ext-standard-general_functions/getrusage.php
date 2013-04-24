<?php

var_dump(gettype(getrusage()));
var_dump(gettype(getrusage(1)));
var_dump(gettype(getrusage(-1)));
var_dump(getrusage(array()));


echo "Done\n";
?>