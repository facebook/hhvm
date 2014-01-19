<?php

function p(array $i = null) {
  var_dump($i);
  $i = array();
}
p();
function q() {
  p(null);
}
