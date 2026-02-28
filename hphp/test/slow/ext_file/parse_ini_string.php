<?hh


<<__EntryPoint>>
function main_parse_ini_string() :mixed{
$inis = dict[
  'general_ini_with_sections' =>
    ';;; Created on Tuesday, October 27, 2009 at 12:01 PM GMT'."\n".
    '[GJK_Browscap_Version]'."\n".
    'Version=4520'."\n".
    'Released=Tue, 27 Oct 2009 12:01:07 -0000'."\n".
    "\n".
    "\n".
    ';;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DefaultProperties'."\n".
    "\n".
    '[DefaultProperties]'."\n".
    'Browser="DefaultProperties"'."\n".
    'Version=0'."\n".
    'Platform=unknown'."\n".
    'Beta=false'."\n",
  'comment_on_last_line' =>
    'key=value'."\n".
    '; comment on last line',
  'double_quote_escape' =>
    'key1="value"'."\n".
    'key2="value""value2"'."\n".
    'key3="value\""'."\n".
    'key4="value""\""'."\n".
    'key5="value\"inner value\""'."\n",
  'joomla_bug' =>
    'JERROR_COULD_NOT_FIND_TEMPLATE="Could not find template ""\""%s""\"""."',
  'slash_in_key' =>
    '[global]'."\n".
    'key1 = "value1"'."\n".
    'path.a.b.c.key2 = "value2"'."\n".
    'path.a/b/c.key3 = "value3"'."\n",
  'dollar_sign_at_end' =>
    '[global]'."\n".
    'fonts.trackingUrl = "https://fast.fonts.com/t/1.css?apiType=css&projectid=123"'."\n".
    'serviceCache.1.cacheable = "true"'."\n".
    'anotherkey = "value$value"'."\n".
    'somekey = "value$"',
  'negative_number_as_key' =>
    '[FORMERRORCODES]'."\n".
    '-1= "SimpleError"',
  'double slash' =>
    'key1 = "start\\\\"'."\n".
    'key2 = "\\\\end"'."\n".
    'key3 = "start\\\\end"',
];

foreach ($inis as $key => $ini) {
  var_dump($key);
  echo "===\n";
  var_dump(parse_ini_string($ini));
  echo "===\n";
  var_dump(parse_ini_string($ini, true));
  echo "===\n";
}
}
