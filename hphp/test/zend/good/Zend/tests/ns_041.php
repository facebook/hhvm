<?php
namespace test\ns1;

const FOO = "ok\n";
  
echo(FOO);
echo(\test\ns1\FOO);
echo(\test\ns1\FOO);
echo(BAR);

const BAR = "ok\n";
