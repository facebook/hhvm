<?php

Namespace FOO;

FUNCTION test() {
  ECHO "YOU ARE HEARING ME TALK\n";
}

NAMESPACE BAR;
USE FOO As BAZ;

<<__EntryPoint>>
function main_namespace() {
BAZ\test();
}
