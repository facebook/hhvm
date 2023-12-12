<?hh
<<__EntryPoint>>
function main_entry(): void {
  mb_regex_encoding('iso-8859-1');
  $test_str = "I\xf1t\xebrn\xe2ti\xf4n\xe0liz\xe6ti\xf8n";

  if(mb_ereg_search_init($test_str))
  {
  	$val=mb_ereg_search_pos("n\xe2ti\xf4n");

  	var_dump($val);

  }
  else{
  	var_dump("false");
  }
}
