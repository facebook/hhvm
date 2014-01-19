<?php
apc_store("ts", 12);
if (apc_dec("ts") !== 11) echo "no\n";
if (apc_dec("ts", 5) !== 6) echo "no\n";
if (apc_dec("ts", -3) !== 9) echo "no\n";
echo "ok\n";

