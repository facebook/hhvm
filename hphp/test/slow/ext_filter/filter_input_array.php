<?php
$result = filter_input_array(INPUT_SERVER, FILTER_UNSAFE_RAW);
if ($result === false) {
  echo "ERROR: result is false\n";
  exit;
}

if ($result['DOCUMENT_ROOT'] !== '') {
  echo "ERROR: DOCUMENT_ROOT is not empty string\n";
  exit;
}

echo $result['PHP_SELF'] . "\n";
echo $result['SCRIPT_NAME'] . "\n";
echo $result['SCRIPT_FILENAME'] . "\n";
