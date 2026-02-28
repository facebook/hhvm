<?hh

function good_func() :mixed{
  echo 'this doen\'t print';
  return 'nor this';
}

<<__EntryPoint>>
function main_good_func() :mixed{
ob_start(good_func<>);
echo 'this doen\'t print';
var_dump(ob_end_clean());
}
