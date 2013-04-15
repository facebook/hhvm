<?php

function a() {
  $a = static function() { var_dump(true); };
  $a->__invoke();
}
a();
