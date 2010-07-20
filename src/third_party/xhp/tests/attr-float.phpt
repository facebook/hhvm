--TEST--
Float attribute parsing
--FILE--
<?php
class :foo {
  attribute
    float b;
}
echo "pass";
--EXPECT--
pass
