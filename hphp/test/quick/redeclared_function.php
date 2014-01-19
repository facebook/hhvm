<?php

echo "declaring f()\n";
function f() { echo "first def!\n"; }
echo "re-declaring f()\n";
function f() { echo "second def!\n"; }
f();

