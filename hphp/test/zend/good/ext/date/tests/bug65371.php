<?php

function p($str)
{
  echo $str, "\n";
  echo strftime($str), "\n";
  echo bin2hex($str), "\n";
  echo bin2hex(strftime($str));
}

setlocale(LC_ALL, 'C');
p('あ');
