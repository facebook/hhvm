<?hh <<__EntryPoint>> function main(): void {
echo md5("")."\n";
echo md5("a")."\n";
echo md5("abc")."\n";
echo md5("message digest")."\n";
echo md5("abcdefghijklmnopqrstuvwxyz")."\n";
echo md5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")."\n";
echo md5("12345678901234567890123456789012345678901234567890123456789012345678901234567890")."\n";
}
