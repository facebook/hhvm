<?php 
function test($s) {
  echo "'".trim(str_replace("&nbsp;", " ", htmlspecialchars_decode(strip_tags(highlight_string($s,1)))))."'\n";
}

eval('echo "1";//2');
eval('echo 3; //{ 4?>5');
echo "\n";

//test('<?php echo "1";//');
test('<?php echo "1";//2');
test('<?php echo "1";//22');
test('<?php echo 3; // 4 ?>5');
?>