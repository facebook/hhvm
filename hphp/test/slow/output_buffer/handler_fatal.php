<?hh

function main() :mixed{
  ob_start(function($str) {
    bar();
    return $str.'!!!';
  });

  echo 'garbanzo beans';

  ob_end_flush();

  echo "DON'T PRINT ME!";
}


<<__EntryPoint>>
function main_handler_fatal() :mixed{
main();
}
