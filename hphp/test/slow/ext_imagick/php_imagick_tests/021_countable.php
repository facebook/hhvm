<?hh

<<__EntryPoint>> function main(): void {
$imagick = new Imagick(vec [
            'magick:rose',
            'magick:rose',
            'magick:rose',
]);

echo count ($imagick) . PHP_EOL;
echo 'done' . PHP_EOL;
}
