<?hh

<<__EntryPoint>> function main(): void {
print <<<ENDOFHEREDOC
ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
ENDOFHEREDOC;

print "{$x}";
}
