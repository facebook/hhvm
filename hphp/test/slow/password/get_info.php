<?php

function print_info($info) {
  echo $info['algo']."\n";
  echo $info['algoName']."\n";
  if (isset($info['options']['cost']))
    echo $info['options']['cost']."\n";
  echo "====\n";
}

print_info(password_get_info('foo'));
print_info(password_get_info('$2y$'));
print_info(password_get_info(
             '$2y$07$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi'));
print_info(password_get_info(
             '$2y$10$usesomesillystringfore2uDLvp1Ii2e./U9C8sBjqp8I90dH6hi'));
