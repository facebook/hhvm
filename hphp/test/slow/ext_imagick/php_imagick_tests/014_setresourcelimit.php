<?hh
<<__EntryPoint>> function main(): void {
Imagick::setResourceLimit(Imagick::RESOURCETYPE_MEMORY, 64);

echo 'success';
}
