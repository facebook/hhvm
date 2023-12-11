<?hh

function f(): dict {
  $str = dict[];
  $str[0] = "The quick brown fox jumped over the lazy dog.";
  $str[1] = "The quick brown fox jumped over the quick brown fox.";
  $patterns = dict[];
  $replacements = dict[];
  $patterns[0] = "/quick/";
  $patterns[1] = "/brown/";
  $patterns[2] = "/fox/";
  $replacements[2] = "bear";
  $replacements[1] = "black";
  $replacements[0] = "slow";
  // Without ksort, replacements will come out, "bear black slow"
  $result = preg_replace($patterns, $replacements, $str);
  var_dump($result);
  var_dump(HH\is_dict($result));
  return $result;
}


<<__EntryPoint>>
function main_preg_replace_impl_type() :mixed{
f();
}
