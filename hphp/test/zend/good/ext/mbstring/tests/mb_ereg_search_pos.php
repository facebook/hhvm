<?hh
<<__EntryPoint>>
function main_entry(): void {
  mb_regex_encoding('iso-8859-1');
  $test_str = 'I�t�rn�ti�n�liz�ti�n';

  if(mb_ereg_search_init($test_str))
  {
  	$val=mb_ereg_search_pos("n�ti�n");

  	var_dump($val);

  }
  else{
  	var_dump("false");
  }
}
