<?hh
<<__EntryPoint>> function main(): void {
try {
} catch (A $e) {
}

try {
  throw new Exception();
} catch (B $e) {
} catch (Exception $e) {
    echo "ok\n";
}
}
