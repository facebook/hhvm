<?hh

class foo {
    public $bar = <<<'EOT'
bar
EOT;
}

<<__EntryPoint>> function main(): void {
print "ok!\n";
}
