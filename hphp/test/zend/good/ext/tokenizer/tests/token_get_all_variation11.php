<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Using different control structure keywords 
 *   if..else, elseif - T_IF(301), T_ELSEIF(302), T_ELSE(303)
 *   while - T_WHILE(318)
 *   do...while - T_DO(317)
 *   for - T_ENDFOR(320)
 *   foreach - T_ENDFOREACH(322)
 *   switch...case - T_ENDSWITCH(327), T_CASE(329)
 *   break - T_BREAK(331)
 *   continue - T_CONTINUE(332)
*/

echo "*** Testing token_get_all() : for control structure tokens ***\n";

// if..elseif....else
echo "-- with if..elseif..else..tokens --\n";

$source = '<?php 
if($a == true) {
     echo "$a = true";
}
elseif($a == false) {
  echo false;
}
else
  echo 1;
?>';

var_dump( token_get_all($source));

// while..., do..while, break, continue
echo "-- with while..., do..while, switch & continue tokens --\n";

$source = "<?php while(true) {
echo 'True';
break;
}
do {
continue;
}while(false); ?>";

var_dump( token_get_all($source));

// for..., foreach( as )
echo "-- with for..foreach( as ) tokens --\n";

$source = '<?php for($count=0; $count < 5; $count++) {
echo $count;
}
foreach($values as $index) {
continue;
} ?>';

var_dump( token_get_all($source));

// switch..case, default
echo "-- with switch...case tokens --\n";

$source = '<?php switch($var)
case 1: break;
default: echo "default"; ?>';

var_dump( token_get_all($source));

echo "Done"
?>
