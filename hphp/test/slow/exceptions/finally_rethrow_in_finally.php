<?php

try {
  throw new Exception("muahahhaha.");
} finally {
  $e = null;
  try {
    $e = new Exception();
    throw $e;
  } finally {
    throw $e;
  }
}
