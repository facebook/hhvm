<?php

function t($name) {
  if (!defined($name)) define($name, $name .':'. $name);
  var_dump(constant($name));
}


<<__EntryPoint>>
function main_sid() {
t('SID');
t('I_LOVE_PONIES');
}
