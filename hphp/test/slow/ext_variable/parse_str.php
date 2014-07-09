<?php
define('EOL',"\n");

$data='a=1&b=2&c=3';
echo 'global scope',EOL;
echo 'parse_data(): ',$data,EOL;
parse_str($data);
if (isset($b)) {
  echo 'isset b='.$b,EOL;
} else {
  echo 'no b='.$b,EOL;
}
echo EOL;

echo 'within function',EOL;
testme();
function testme() {
  $data='a=1&b=2&c=3';
  echo 'parse_data(): ',$data,EOL;
  parse_str($data);
  if (isset($b)) {
    echo 'isset b='.$b,EOL;
  } else {
    echo 'notset b='.$b,EOL;
  }
  if (isset($b)) {
    echo 'isset b='.$b,EOL;
  } else {
    echo 'no b='.$b,EOL;
  }
}
