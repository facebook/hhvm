<?php
  try {
  } finally {
    try {
       goto foo;
    }
    finally {}
    foo:
  }
?>
