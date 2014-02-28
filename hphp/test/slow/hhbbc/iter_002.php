<?php

function foo($x) {
  foreach ($x as $k) yield $k;
}
