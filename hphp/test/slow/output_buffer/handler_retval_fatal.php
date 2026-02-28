<?hh

class Chickpea {
  public function __toString() :mixed{
    chickpea();
    return 'chickpea';
  }
}

function main() :mixed{
  ob_start(function($str) {
    return new Chickpea();
  });

  echo 'garbanzo beans';

  ob_end_flush();

  echo "DON'T PRINT ME!";
}


<<__EntryPoint>>
function main_handler_retval_fatal() :mixed{
main();
}
