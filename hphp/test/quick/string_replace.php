<?hh

function replace_array_str($in) {
  $search = array('a', 'b');
  $count = 0;
  $out = str_replace_with_count($search, '', $in, inout $count);
  var_dump(array($out, $count));
}

function replace_array_array($inarr) {
  $search = array('a', 'b');
  $count = 0;
  $out = str_replace_with_count($search, '', $inarr, inout $count);
  var_dump(array($out, $count));
}

function replace_str_str($in) {
  $search = 'a';
  $count = 0;
  $out = str_replace_with_count($search, '', $in, inout $count);
  var_dump(array($out, $count));
}

<<__EntryPoint>> function main(): void {
  replace_array_str('a');
  replace_array_str('b');
  replace_array_str('ab');
  replace_array_str('xabx');
  replace_array_str('xaaabx');

  replace_array_array(array('a', 'b'));
  replace_array_array(array('a', 'x'));
  replace_array_array(array('x', 'a'));
  replace_array_array(array('xxabax', 'xxbx'));

  replace_str_str('a');
  replace_str_str('x');
  replace_str_str('xaxax');
}
