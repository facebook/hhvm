<?hh // strict

namespace NS_nullables_in_types_dir;

class Button {}
class CustomButton extends Button {}

interface MyCollection {}
class MyList implements MyCollection {}
class MyQueue implements MyCollection {}

class C1 {
  private ?bool $pr_nbool = null;
  private ?int $pr_nint = null;
  private ?float $pr_nfloat = null;
  private ?num $pr_nnum = null;
  private ?string $pr_nstring = null;
  private mixed $pr_mixed = null;	// ?mixed === mixed; ? is redundant
  private ?resource $pr_nresource = null;

  private ?Button $pr_nButton = null;
  private ?CustomButton $pr_nCustomButton = null;
  private ?MyCollection $pr_nMyCollection = null;

///*
  private array<?bool> $a_nbool = array(true, null);
  private array<?int> $a_nint = array(10, null);
  private array<?float> $a_nfloat = array(3.1, null);
  private array<?num> $a_nnum = array(-4, 5.4, null);
  private array<?string> $a_nstring = array('a', null);
  private array<mixed> $a_mixed = array(10, 'b', null);	// ?mixed === mixed; ? is redundant
//  private array<?resource> $a_nresource = array(STDIN, null);

  private array<?Button> $a_nButton = array();
  private array<?CustomButton> $a_nCustomButton = array();
  private array<?MyCollection> $a_nMyCollection = array();
//*/

  public function __construct() {
    echo "Inside " . __METHOD__ . "\n";

///*
    $this->pr_nbool = true;
    $this->pr_nint = 10;
    $this->pr_nfloat = 3.1;
    $this->pr_nnum = -4; $this->pr_nnum = 5.4;
    $this->pr_nstring = 'a';
    $this->pr_mixed = 10; $this->pr_mixed = 'b';
//    $this->pr_nresource = STDIN;

    $this->pr_nButton = new Button();
    $this->pr_nButton = new CustomButton();
    $this->pr_nCustomButton = new CustomButton();
    $this->pr_nButton = $this->pr_nCustomButton;
    $this->pr_nCustomButton = $this->pr_nButton;

    $this->pr_nMyCollection = new MyList();
    $this->pr_nMyCollection = new MyQueue();
//*/

///*
    $this->a_nbool = array(true, null);
    $this->a_nint = array(10, null);
    $this->a_nfloat = array(3.1, null);
    $this->a_nnum = array(-4, 5.4, null);
    $this->a_nstring = array('a', null);
    $this->a_mixed = array(10, 'b', null);
//    $this->a_nresource = array(STDIN, null);

    $this->a_nButton = array(new Button(), null, new CustomButton());
    $this->a_nCustomButton = array(new CustomButton(), null);
    $this->a_nMyCollection = array(null, new MyList(), new MyQueue());
//*/
  }
}

function main(): void {
  $c1 = new C1();
  var_dump($c1);
}

/* HH_FIXME[1002] call to main in strict*/
main();
