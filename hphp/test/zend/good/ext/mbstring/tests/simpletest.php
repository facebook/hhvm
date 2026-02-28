<?hh

<<__EntryPoint>>
function main_entry(): void {
  /*
   * Test basic PHP functions to check if it works with multi-byte chars
   */

  // EUC-JP strings
  $s1 = "\xa5\xde\xa5\xeb\xa5\xc1\xa5\xd0\xa5\xa4\xa5\xc8\xb4\xd8\xbf\xf4\xa4\xac\xbb\xc8\xa4\xa8\xa4\xde\xa4\xb9\xa1\xa3";
  $s2 = "\xa4\xb3\xa4\xce\xca\xb8\xbb\xfa\xa4\xac\xcf\xa2\xb7\xeb\xa4\xb5\xa4\xec\xa4\xc6\xa4\xa4\xa4\xeb\xa4\xcf\xa4\xba\xa1\xa3";

  // print directly
  echo "echo: ".$s1.$s2."\n";
  print("print: ".$s1.$s2."\n");
  printf("printf: %s%s\n",$s1, $s2);
  echo sprintf("sprintf: %s%s\n",$s1, $s2); 

  // Assign to var
  $s3 = $s1.$s2."\n";
  echo "echo: ".$s3;
}
