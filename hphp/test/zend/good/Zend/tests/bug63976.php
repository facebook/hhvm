<?php
if (1) {
  include 'bug63976-1.inc';
}
if (1) {
  include 'bug63976-2.inc';
}
$bar = new Bar();
var_dump($bar->table);
