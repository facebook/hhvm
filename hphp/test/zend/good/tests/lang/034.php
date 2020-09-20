<?hh
# activate the german locale
<<__EntryPoint>> function main(): void {
setlocale(LC_NUMERIC, "de_DE.UTF-8", "de_DE", "de", "german", "ge", "de_DE.ISO-8859-1");
echo (float)"3.14", "\n";
}
