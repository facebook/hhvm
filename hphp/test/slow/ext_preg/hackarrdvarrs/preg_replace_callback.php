<?php

function preg_replace_callback_main() {
  var_dump(preg_replace_callback(
    '/(a)/',
    function ($matches) {
      return str_repeat($matches[0], 500);
    },
    'aaaaaaaaaaaaaaaaaaaa'
  ));
}


<<__EntryPoint>>
function main_preg_replace_callback() {
preg_replace_callback_main();
}
