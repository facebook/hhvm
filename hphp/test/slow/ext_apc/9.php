<?php
apc_store("ts", 12);
apc_cas("ts", 12, 15);
if (apc_fetch("ts") !== 15) echo "no\n";
apc_cas("ts", 12, 18);
if (apc_fetch("ts") !== 15) echo "no\n";
echo "ok\n";
