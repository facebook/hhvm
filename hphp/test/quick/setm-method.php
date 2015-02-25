<?php


function main($o) {
  $o = $o->circle = $o;
  $o->foo();
}
main(new stdclass);
