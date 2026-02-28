<?hh
<<__EntryPoint>>
function entrypoint_msgfmt_format_simple_types_numeric_strings(): void {
  date_default_timezone_set('Atlantic/Azores');
  ini_set("intl.error_level", E_WARNING);
  //ini_set("intl.default_locale", "nl");

  $mf = new MessageFormatter('en_US',"
	none			{a}
	number			{b,number}
	number integer	{c,number,integer}
	number currency	{d,number,currency}
	number percent	{e,number,percent}
	date			{f,date}
	time			{g,time}
	spellout		{h,spellout}
	ordinal			{i,ordinal}
	duration		{j,duration}
	");

  $ex = "1336317965.5 str";
  var_dump($mf->format(dict[
  'a' => $ex,
  'b' => $ex,
  'c' => $ex,
  'd' => $ex,
  'e' => $ex,
  'f' => "  1336317965.5",
  'g' => "  1336317965.5",
  'h' => $ex,
  'i' => $ex,
  'j' => $ex,
  ]));
  echo "==DONE==";
}
