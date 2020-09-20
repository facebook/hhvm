<?hh
<<__EntryPoint>> function main(): void {
echo Locale::canonicalize("test") . "\n";
echo intl_get_error_message() . "\n";
echo Locale::canonicalize(str_repeat("a", 81)) . "\n";
echo intl_get_error_message() . "\n";
}
