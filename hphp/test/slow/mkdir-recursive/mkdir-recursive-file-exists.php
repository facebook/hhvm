<?php

@unlink("foo");
fopen("foo", "w");
var_dump(@mkdir("foo", 777, true));
unlink("foo");
