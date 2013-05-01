<?hh
class C1 {
  private int $pp;
  private string $pField;
  public string $field;
  protected int $field1;
  public static int $sField;
}
$rc = new ReflectionClass('C1');
$rp = $rc->getProperty('pp');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('pField');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('field');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('field1');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('sField');
var_dump($rp->getTypeText());
class C2 extends C1 {
  public int $field;
  private int $pField;
}
$rc = new ReflectionClass('C2');
$rp = $rc->getProperty('field');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('field1');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('pField');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('sField');
var_dump($rp->getTypeText());
var_dump($rc->hasProperty('pp'));
trait T {
  public C1 $tField;
  private C2 $tField1;
}
class TI {
  use T;
  public string $tField;
}
$rc = new ReflectionClass('TI');
$rp = $rc->getProperty('tField');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('tField1');
var_dump($rp->getTypeText());
