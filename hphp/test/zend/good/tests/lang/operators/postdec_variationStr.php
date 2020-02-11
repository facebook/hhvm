<?hh

$strVals = varray["0","65","-44", "1.2", "-7.7", "123e5"];


foreach ($strVals as $strVal) {
   echo "--- testing: '$strVal' ---\n";
   $strVal--;
   var_dump($strVal);
}

echo "===DONE===\n";
