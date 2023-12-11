<?hh
<<__EntryPoint>> function main(): void {
$strVals = vec[
   "0","65","-44", "1.2", "-7.7", "abc", "123abc", "123e5", "123e5xyz", " 123abc", "123 abc", "123abc ", "3.4a",
   "a5.9"
];


foreach ($strVals as $strVal) {
   echo "--- testing: '$strVal' ---\n";
   var_dump(bin2hex(~$strVal));
}
   
echo "===DONE===\n";
}
