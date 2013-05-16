<?php
$array1 = array(0.0, 1, 2.5);
$array2 = array(TRUE, FALSE, NULL, "d", "e", "f");
foreach($array1 as $start)
{
   foreach($array1 as $num)
   {
      foreach($array2 as $value)
      {
         echo '==========================='."\n";
	 echo 'start: '.$start.' num: '.$num.' value: '.$value."\n";
  	 $output = array_fill($start, $num, $value);
	 var_dump($output);
      }
   }
}
echo '== Done ==';
?>
===============Done====================