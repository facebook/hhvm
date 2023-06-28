<?hh

class Name {
  function __construct($_name) {
    $this->name = $_name;
  }

  function display() :mixed{
    echo $this->name . "\n";
  }
}

class Person {
  private $name;

  function __construct($_name, $_address) {
    $this->name = new Name($_name);
  }

  function getName() :mixed{
    return $this->name;
  }
}

<<__EntryPoint>>
function main() :mixed{
  $person = new Person("John", "New York");
  $person->getName()->display();
}
