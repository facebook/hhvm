<?hh
class mytest
{
  const classConstant = '01';

  private $classArray = dict[ mytest::classConstant => 'value' ];

  public function __construct()
  {
    print_r($this->classArray);
  }
}

const normalConstant = '01';

<<__EntryPoint>> function main(): void {
$classtest = new mytest();

$normalArray = dict[ normalConstant => 'value' ];
print_r($normalArray);
}
