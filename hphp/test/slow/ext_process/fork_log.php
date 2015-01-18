<?php

var_dump($g);
if (!pcntl_fork()) {
  var_dump($g);
}
