<?hh

class base
{
  private $member;

  function __construct()
  {
      echo __METHOD__ . "(begin)\n";
    $this->member = 'base::member';
    $this->test();
      echo __METHOD__ . "(end)\n";
  }

  function test()
:mixed  {
      echo __METHOD__ . "\n";
    print_r($this);
  }
}

class derived extends base
{
  public $member = 'derived::member (default)';

  function __construct()
  {
      echo __METHOD__ . "(begin)\n";
      parent::__construct();
      parent::test();
      $this->test();
    $this->member = 'derived::member';
      echo __METHOD__ . "(end)\n";
  }

  function test()
:mixed  {
      parent::test();
      echo __METHOD__ . "\n";
    print_r($this);
  }
}
<<__EntryPoint>> function main(): void {
$t = new derived;
$t->test();
unset($t);

echo "Done\n";
}
