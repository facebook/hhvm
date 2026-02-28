<?hh
class User
{
    public $username;

    public function __construct($username)
    {
        $this->username = $username;
    }
}


<<__EntryPoint>>
function main_array_column_on_objects() :mixed{
$users = vec[
    new User('user 1'),
    new User('user 2'),
    new User('user 3'),
];

print_r(array_column($users, 'username'));
}
