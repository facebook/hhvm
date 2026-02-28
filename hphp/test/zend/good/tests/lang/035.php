<?hh
class MyException extends Exception {
  function __construct($_error) {
    $this->error = $_error;
  }

  function getException() :mixed{
    return $this->error;
  }
}

function ThrowException() :mixed{
  throw new MyException("'This is an exception!'");
}

<<__EntryPoint>>
function main() :mixed{
  try {
  } catch (MyException $exception) {
    print "There shouldn't be an exception: " . $exception->getException();
    print "\n";
  }

  try {
	  ThrowException();
  } catch (MyException $exception) {
    print "There was an exception: " . $exception->getException();
    print "\n";
  }
}
