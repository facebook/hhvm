<?php

function thr($doit) {
  try {
    var_dump("try");
    if ($doit) {
      throw new Exception('done');
    }
  } catch (Exception $e) {
    var_dump("catch");
  } finally {
    var_dump("finally");
  }
}
thr(true);
thr(false);
