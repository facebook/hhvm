<?hh

class e {

    const E = <<<'THISMUSTNOTERROR'
If you DON'T see this, something's wrong.
THISMUSTNOTERROR;

};

<<__EntryPoint>> function main(): void {
print e::E . "\n";
}
