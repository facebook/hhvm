<?hh
class foo
{
    private $private = 'private';
    protected $protected = 'protected';
    public $public = 'public';
}

class bar extends foo
{
    public function __sleep()
:mixed    {
        return vec["\0foo\0private", 'protected', 'public'];
    }
}
<<__EntryPoint>> function main(): void {
var_dump(str_replace("\0", '\0', serialize(new bar())));
}
