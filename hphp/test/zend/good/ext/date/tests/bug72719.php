<?hh
<<__EntryPoint>> function main(): void {
echo (new DateTimeImmutable('Monday next week 13:00'))->format('l'), "\n";
echo (new DateTimeImmutable('Tuesday next week 14:00'))->format('l'), "\n";
echo (new DateTimeImmutable('Wednesday next week 14:00'))->format('l'), "\n";
echo (new DateTimeImmutable('Thursday next week 15:00'))->format('l'), "\n";
echo (new DateTimeImmutable('Friday next week 16:00'))->format('l'), "\n";
echo (new DateTimeImmutable('Saturday next week 17:00'))->format('l'), "\n";
echo (new DateTimeImmutable('Sunday next week 18:00'))->format('l'), "\n";
}
