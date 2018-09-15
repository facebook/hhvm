<?php

$e = null;
try {
  $e = new Exception("Oh my!");
  throw $e;
} finally {
  throw $e;
}
