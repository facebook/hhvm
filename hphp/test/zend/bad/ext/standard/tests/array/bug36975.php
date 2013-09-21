<?php
$a = array('aa', 'aa', 'bb', 'bb', 'cc', 'cc');                  
$test = natcasesort($a);
if ($test) {                                                            
  echo "natcasesort success!\n";                                        
}                                                                       
$val = array_pop($a);                                            
$a[] = $val;                                                     
var_dump($a);

$b = array(1 => 'foo', 0 => 'baz');
array_pop($b);
$b[] = 'bar';
array_push($b, 'bar');
print_r($b);

$c = array(0, 0, 0, 0, 0);
asort($c);
array_pop($c);
$c[] = 'foo';
$c[] = 'bar';
var_dump($c);
?>