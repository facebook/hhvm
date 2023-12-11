<?hh <<__EntryPoint>> function main(): void {
$valid_true = vec[1, "1", "true", 1.0];
$valid_false = vec[0, "", 0.0, NULL];

$int1 = 679;
$int2 = -67835;
$valid_int1 = vec["679", "679abc", " 679", 679.0, 6.79E2, "678", "678abc", " 678", 678.0, 6.78E2, 6.789E2, "+678", +678];
$valid_int2 = vec["-67835", "-67835abc", " -67835", -67835.000, -6.7835E4, "-67836", "-67836abc". " -67836", -67835.0001, -6.78351E4, "-67836", -67835.0001, -6.78351E4];
$invalid_int1 = vec[680, "680"];
$invalid_int2 = vec[-67834, "-67834"];

$float1 = 57385.45835;
$float2 = -67345.76567;
$valid_float1 = vec["57385.45835",  "57385.45835aaa", "  57385.45835", 5.738545835e4, "57385.45834",  "57385.45834aaa", "  57385.45834", 5.738545834e4];
$valid_float2 = vec["-67345.76567", "-67345.76567aaa", "  -67345.76567", -6.734576567E4, "-67345.76568", "-67345.76568aaa", "  -67345.76568", -6.734576568E4];
$invalid_float1 = vec[57385.45836, 5.738545836e4];
$invalid_float2 = vec[-67345.76564, -6.734576564E4];

$toCompare = vec[

  true, array_merge($valid_false, $valid_true), NULL,
  false, $valid_false, $valid_true,
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
      if (HH\Lib\Legacy_FIXME\gte($typeToTest, $compareVal)) {
         // do nothing
      }
      else {
         echo "FAILED: '$typeToTest' < '$compareVal'\n";
         $failed = true;
      }
   }

   if ($invalid_compares != NULL) {
       foreach($invalid_compares as $compareVal) {
          if (HH\Lib\Legacy_FIXME\gte($typeToTest, $compareVal)) {
             echo "FAILED: '$typeToTest' >= '$compareVal'\n";
             $failed = true;
          }
       }
   }

}
if ($failed == false) {
   echo "Test Passed\n";
}
echo "===DONE===\n";
}
