<?php

function a() {
  return function() { return $this->b; };
}
$c = a();
$c();
