<?hh <<__EntryPoint>> function main(): void {
DateTime::createFromFormat('s', '0');
$lastErrors = DateTime::getLastErrors();
print_r($lastErrors['errors'][0]);
}
