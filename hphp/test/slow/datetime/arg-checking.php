<?hh

class X {}

function test($fn) {
  try {
    var_dump($fn());
  } catch (Exception $e) {
    echo "Warning: ".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main_arg_checking(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  $utc = new \DateTimeZone('UTC');
  $amsterdam = new \DateTimeZone('Europe/Amsterdam');
  $date = new \DateTime("2012-10-10");

  test(() ==> DateTime::createFromFormat('j-M-Y', '15-Feb-2009', new X()));
  test(() ==> $date->setTimezone(new X()));
  test(() ==> $amsterdam->getOffset($utc));
  test(() ==> $amsterdam->getOffset($date));
  test(() ==> $date->add($utc));
  test(() ==> $date->sub($utc));
  test(() ==> $date->diff(new X()));
  test(() ==> date_format(new X(), "Y"));
  test(() ==> date_create("2012-10-10", new X()));
}
