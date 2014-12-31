<?php
var_dump(ezc_try_call(function () {
  try {
    ezc_throw('Exception');
  } catch ( Exception $e ) {}
  return false;
}));
