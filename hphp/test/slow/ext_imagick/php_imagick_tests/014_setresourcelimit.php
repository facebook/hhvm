<?php
<<__EntryPoint>> function main() {
Imagick::setResourceLimit(Imagick::RESOURCETYPE_MEMORY, 64);

echo 'success';
}
