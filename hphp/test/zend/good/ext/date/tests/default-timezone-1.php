<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('GMT');
    putenv('TZ='); // clean TZ so that it doesn't bypass the ini option
    echo strtotime("2005-06-18 22:15:44");
}
