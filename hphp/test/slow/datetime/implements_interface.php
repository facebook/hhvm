<?php
date_default_timezone_set('UTC');

function main() {
  var_dump((new DateTime()) instanceof DateTimeInterface);
}

main();
