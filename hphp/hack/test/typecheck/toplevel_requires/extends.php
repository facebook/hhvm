<?hh // strict

require_once('Bar.php');

// Moving the require to an <<__EntryPoint>> function leads to a runtime fatal
// at the class definition, before the __EntryPoint function is invoked
class Foo extends Bar {}
