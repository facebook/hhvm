<?php

function apc_apache_note() {
  apache_note('what', apc_fetch('what'));
  apc_store('what', 'hello' . rand(100));
}

apc_apache_note();
var_dump('OK');
