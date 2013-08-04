<?php

async function f1($a) {
  return "f1, ".$a;
}

async function f2($a) {
  return await f1("f2, ".$a);
}

async function f3($a) {
  return await f2("f3, ".$a);
}

$r = f3("var_dump.")->join();
var_dump($r);
