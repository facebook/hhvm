<?php

apc_store("ts", "TestString");
apc_store("ta", array("a" => 1, "b" => 2));
apc_delete("ts");
apc_delete("ta");
if (apc_fetch("ts") !== false) echo "no\n";
if (apc_fetch("ta") !== false) echo "no\n";

if (apc_fetch("ts") !== false) echo "no\n";
if (apc_fetch("ta") !== false) echo "no\n";

apc_store("ts", "TestString");
apc_store("ta", array("a" => 1, "b" => 2));
if (apc_delete(array("ts", "ta")) !== array()) echo "no\n";
if (apc_fetch("ts") !== false) echo "no\n";
if (apc_fetch("ta") !== false) echo "no\n";
echo "ok\n";
