<?php

class X { protected $x = __DIR__; }class Y { protected $y = __FILE__; }
<<__EntryPoint>>
function main_deepinit_file_dir() {
; new X;
; new Y;
echo "ok\n";
}
