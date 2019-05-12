<?php
interface IA {
    const FOO = 10;
}

interface IB extends IA {
}

class C implements IA, IB {
}
<<__EntryPoint>> function main() {
echo "Done\n";
}
