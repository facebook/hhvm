<?hh

trait Company {
  public function getName() :mixed{
    return 'Facebook';
  }
}
class English {
  use Company;
  public function getHi() :mixed{
    return "Hi ";
  }
  public function sayHello() :mixed{
    echo $this->getHi() . $this->getName();
  }
}
<<__EntryPoint>> function main(): void {
$e = new English();
$e->sayHello();
}
