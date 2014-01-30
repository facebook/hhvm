<?php

class X { protected $x = __DIR__; }; new X;
class Y { protected $y = __FILE__; }; new Y;
echo "ok\n";
