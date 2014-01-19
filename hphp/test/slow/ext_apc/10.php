<?php
apc_store("ts", "TestString");
if (apc_exists("ts") !== true) echo "no\n";
if (apc_exists("TestString") !== false) echo "no\n";
if (apc_exists(array("ts", "TestString") !== array("ts"))) echo "no\n";
echo "ok\n";
