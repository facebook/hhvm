<?php

apc_store("ts", 12);
if (apc_inc("ts") !== 13) echo "no\n";
if (apc_inc("ts", 5) !== 18) echo "no\n";
if (apc_inc("ts", -3) !== 15) echo "no\n";
echo "ok\n";
