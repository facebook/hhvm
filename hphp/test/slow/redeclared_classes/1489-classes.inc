<?hh

class Y extends X {
  protected $prot_over_prot = 4;
  public $pub_over_pub = 5;
  public $pub_over_prot = 6;
  protected $prot_base = 7;
  public $pub_base = 8;
}
class Z extends Y {
  public $prot_over_prot = 9;
  public $pub_over_pub = 10;
  public $pub_over_prot = 11;
  public $prot_base = 12;
  public $pub_base = 13;
}
