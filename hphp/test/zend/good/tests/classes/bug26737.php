<?hh
class foo {
  private $private = 'private';
  protected $protected = 'protected';
  public $public = 'public';

  public function __sleep() :mixed{
    return vec['private', 'protected', 'public', 'no_such'];
  }
}

<<__EntryPoint>> function main(): void {
$foo = new foo();
$data = serialize($foo);
var_dump(str_replace("\0", '\0', $data));
}
