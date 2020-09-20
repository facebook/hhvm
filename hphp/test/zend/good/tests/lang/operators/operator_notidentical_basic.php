<?hh
<<__EntryPoint>> function main(): void {
$valid_true = varray[1, "1", "true", 1.0, varray[1]];
$valid_false = varray[0, "", 0.0, varray[], NULL];

$int1 = 679;
$int2 = -67835;
$valid_int1 = varray["679", "679abc", " 679", "679  ", 679.0, 6.79E2, "+679", "6 7 9", "6y79", 678];
$valid_int2 = varray["-67835", "-67835abc", " -67835", "-67835  ", -67835.000, -6.7835E4, "- 67835", "-67,835", "-67 835", "-678y35", -76834];
$invalid_int1 = varray[679, +679];
$invalid_int2 = varray[-67835];

$float1 = 57385.45835;
$float2 = -67345.76567;
$valid_float1 = varray["57385.45835",  "57385.45835aaa", "  57385.45835", "57385. 45835",  "57,385.45835", 57385.45834, 5.738545834e4];
$valid_float2 = varray["-67345.76567", "-67345.76567aaa", "  -67345.76567", "- 67345.76567", "-67,345.76567", -67345.76566, -6.734576566E4];
$invalid_float1 = varray[57385.45835, 5.738545835e4]; 
$invalid_float2 = varray[-67345.76567, -6.734576567E4];


$toCompare = varray[
  true, array_merge($valid_true, $valid_false), varray[true], 
  false, array_merge($valid_true, $valid_false), varray[false],
  $int1, $valid_int1, $invalid_int1,
  $int2, $valid_int2, $invalid_int2,
  $float1, $valid_float1, $invalid_float1,
  $float2, $valid_float2, $invalid_float2
];
  
$failed = false;
for ($i = 0; $i < count($toCompare); $i +=3) {
   $typeToTest = $toCompare[$i];
   $valid_compares = $toCompare[$i + 1];
   $invalid_compares = $toCompare[$i + 2];
   
   foreach($valid_compares as $compareVal) {
      if ($typeToTest !== $compareVal) {
         // do nothing
      }
      else {
         echo "FAILED: '$typeToTest' === '$compareVal'\n";
         $failed = true;
      }
   }
   
   foreach($invalid_compares as $compareVal) {
      if ($typeToTest !== $compareVal) {
         echo "FAILED: '$typeToTest' !== '$compareVal'\n";
         $failed = true;
      }
   }
   
}
if ($failed == false) {
   echo "Test Passed\n";
}
   
echo "===DONE===\n";
}
