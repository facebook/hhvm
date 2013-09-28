<?php

trait Company {
  public function getName() {
    return 'Facebook';
  }
}
trait Person {
  public function getName() {
    return 'Ottoni';
  }
}
class Language{
}
class English extends Language {
  use Company, Person {
    Person::getName insteadof Company;
    Company::getName as getCompanyName;
  }
  public function sayHello() {
    echo "Hello " . $this->getCompanyName() . "\n";
    echo "I'm " . $this->getName() . "\n";
  }
}
class Portuguese extends Language {
  use Company, Person {
    Person::getName insteadof Company;
    Company::getName as getCompanyName;
  }
  public function sayHello() {
    echo "Oi " . $this->getCompanyName() . "\n";
    echo "Eu sou " . $this->getName() . "\n";
  }
}
$e = new English();
$e->sayHello();
$p = new Portuguese();
$p->sayHello();
