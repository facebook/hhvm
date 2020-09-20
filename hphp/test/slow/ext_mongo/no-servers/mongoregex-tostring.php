<?hh <<__EntryPoint>> function main(): void {
$regex = new MongoRegex('/foo[bar]{3}/imx');
echo (string) $regex . "\n";
}
