<?php

interface test1 {
    const FOO = 10;
}

interface test2 {
    const FOO = 10;
}

class test implements test1, test2 {
}
<<__EntryPoint>> function main() {
echo "Done\n";
}
