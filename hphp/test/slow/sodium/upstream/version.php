<?hh <<__EntryPoint>> function main(): void {
echo strlen(SODIUM_LIBRARY_VERSION) >= 5;
echo "\n";
echo SODIUM_LIBRARY_MAJOR_VERSION >= 4;
echo "\n";
echo SODIUM_LIBRARY_MINOR_VERSION >= 0;
}
