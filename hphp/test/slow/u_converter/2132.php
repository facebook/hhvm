<?hh


<<__EntryPoint>>
function main_2132() :mixed{
foreach(vec['?','','??'] as $subst) {
  $opts = dict['to_subst' => $subst];
  $ret = UConverter::transcode("This is an ascii string", 'ascii', 'utf-8', $opts);
  if ($ret === NULL) {
    echo "Error: ", intl_get_error_message(), "\n";
  }
 else {
    var_dump($ret);
  }
  $ret = UConverter::transcode("Snowman: (\xE2\x98\x83)", 'ascii', 'utf-8', $opts);
  if ($ret === NULL) {
    echo "Error: ", intl_get_error_message(), "\n";
  }
 else {
    var_dump($ret);
  }
}
}
