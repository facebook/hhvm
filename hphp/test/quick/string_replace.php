<?hh

function replace_array_str($in) :mixed{
  $search = vec['a', 'b'];
  $count = 0;
  $out = str_replace_with_count($search, '', $in, inout $count);
  var_dump(vec[$out, $count]);
}

function replace_array_array($inarr) :mixed{
  $search = vec['a', 'b'];
  $count = 0;
  $out = str_replace_with_count($search, '', $inarr, inout $count);
  var_dump(vec[$out, $count]);
}

function replace_str_str($in) :mixed{
  $search = 'a';
  $count = 0;
  $out = str_replace_with_count($search, '', $in, inout $count);
  var_dump(vec[$out, $count]);
}

<<__EntryPoint>> function main(): void {
  replace_array_str('a');
  replace_array_str('b');
  replace_array_str('ab');
  replace_array_str('xabx');
  replace_array_str('xaaabx');

  replace_array_array(vec['a', 'b']);
  replace_array_array(vec['a', 'x']);
  replace_array_array(vec['x', 'a']);
  replace_array_array(vec['xxabax', 'xxbx']);

  replace_str_str('a');
  replace_str_str('x');
  replace_str_str('xaxax');
}
