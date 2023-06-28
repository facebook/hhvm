<?hh
class C1 {
  private int $pp;
  private string $pField;
  public string $field;
  protected int $field1;
  public static int $sField;
}
class C2 extends C1 {
  public string $field;
  private int $pField;
}
trait T {
  public C1 $tField;
  private C2 $tField1;
}
class TI {
  use T;
  public string $tField;
}

<<__EntryPoint>>
function main_1364() :mixed{
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
$rc = new ReflectionClass('TI');
$rp = $rc->getProperty('tField');
var_dump($rp->getTypeText());
$rp = $rc->getProperty('tField1');
var_dump($rp->getTypeText());
}
