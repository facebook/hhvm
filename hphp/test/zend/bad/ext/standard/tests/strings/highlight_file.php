<?php

$filename = dirname(__FILE__)."/highlight_file.dat";

var_dump(highlight_file());
var_dump(highlight_file($filename));

var_dump(highlight_file('data:,<?php echo "test"; ?>'));

var_dump(highlight_file('data:,<?php echo "test ?>'));

$data = '
<?php 
 class test { 
	 public $var = 1; 
	 private function foo() { echo "foo"; }
	 public function bar() { var_dump(test::foo()); }
 }  
?>';

file_put_contents($filename, $data);
var_dump(highlight_file($filename));


@unlink($filename);
echo "Done\n";
?>