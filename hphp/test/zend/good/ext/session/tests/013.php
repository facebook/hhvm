<?php
error_reporting(E_ALL);

session_id(uniqid("abtest"));
session_start();
session_destroy();
session_id(uniqid("abtest2"));
session_start();
session_destroy();

print "I live\n";
