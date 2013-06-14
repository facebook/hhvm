<?php

apc_store("ts", "TestString");
apc_store("ta", array("a" => 1, "b" => 2));

apc_clear_cache();
if (apc_fetch("ts") !== false) echo "no\n";
if (apc_fetch("ta") !== false) echo "no\n";

if (apc_fetch("ts") !== false) echo "no\n";
if (apc_fetch("ta") !== false) echo "no\n";

echo "ok\n";
