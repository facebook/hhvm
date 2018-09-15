<?php

function main() {
  var_dump((new DateTime()) instanceof DateTimeInterface);
}

<<__EntryPoint>>
function main_implements_interface() {
date_default_timezone_set('UTC');

main();
}
