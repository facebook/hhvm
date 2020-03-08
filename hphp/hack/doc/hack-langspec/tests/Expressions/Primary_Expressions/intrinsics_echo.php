<?hh // strict

namespace NS_intrinsics_echo;

require_once 'Point.php';

function main(): void {
  $v1 = true;
  $v2 = 123;
  $v3 = 20.3E2;
  $v4 = null;
  $v5 = "Hello";
  $v6 = new \NS_intrinsics_echo\Point(3.0, 5.0);

  echo '>>' . $v1 . '|' . $v2 . "<<\n";
  echo '>>' , $v1 , '|' , $v2 , "<<\n";
  echo ('>>' . $v1 . '|' . $v2 . "<<\n");
  echo (('>>') . ($v1) . ('|') . ($v2) . ("<<\n"));// outer parens are part of optional syntax
							// inner ones are redundant grouping parens
//  echo ('>>' , $v1 , '|' , $v2 , "<<\n");	// parens no allowed with commas

  echo '>>' . $v3 . '|' . $v4 . '|' . $v5 . '|' . $v6 . "<<\n";
  echo '>>' , $v3 , '|' , $v4 , '|' , $v5 , '|' , $v6 , "<<\n";

  $v3 = "qqq{$v2}zzz";
  var_dump($v3);
  echo "$v3\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();