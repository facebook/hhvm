<?hh

function preg_replace_callback_main() :mixed{
  $count = -1;
  var_dump(preg_replace_callback(
    '/(a)/',
    function ($matches) {
      return str_repeat($matches[0], 500);
    },
    'aaaaaaaaaaaaaaaaaaaa',
    -1,
    inout $count,
  ));
}


<<__EntryPoint>>
function main_preg_replace_callback() :mixed{
preg_replace_callback_main();
}
