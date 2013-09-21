<?php 
  setlocale(LC_ALL,"C"); 

  function ctype_test_001($function) {
    $n=0; 	
    for($a=0;$a<256;$a++) {
	    if($function($a)) $n++;
    }
	  echo "$function $n\n";
  }
ctype_test_001("ctype_lower");
ctype_test_001("ctype_upper");
ctype_test_001("ctype_alpha");	
ctype_test_001("ctype_digit");	
ctype_test_001("ctype_alnum");		
ctype_test_001("ctype_cntrl");	
ctype_test_001("ctype_graph");
ctype_test_001("ctype_print");
ctype_test_001("ctype_punct");
ctype_test_001("ctype_space");
ctype_test_001("ctype_xdigit");
?>