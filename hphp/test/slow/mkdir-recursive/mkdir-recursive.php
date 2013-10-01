<?php

@rmdir("foo");
mkdir("foo", 777);
var_dump(@mkdir("foo", 777, true));
rmdir("foo");
