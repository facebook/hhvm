<?hh <<__EntryPoint>> function main(): void {
$values = vec[
'a@b.c',
'abuse@example.com',
'test!.!@#$%^&*@example.com',
'test@@#$%^&*())).com',
'test@.com',
'test@com',
'@',
'[]()/@example.com',
'QWERTYUIOPASDFGHJKLZXCVBNM@QWERTYUIOPASDFGHJKLZXCVBNM.NET',
'e.x.a.m.p.l.e.@example.com',
'firstname.lastname@employee.2something.com',
'-@foo.com',
'foo@-.com',
'foo@bar.123',
'foo@bar.-'
];
foreach ($values as $value) {
    var_dump(filter_var($value, FILTER_VALIDATE_EMAIL));
}

echo "Done\n";
}
