<?hh <<__EntryPoint>> function main(): void {
$h = new SplMinHeap();
try {
    $h->top();
} catch (Exception $e) {
    echo $e->getMessage();
}
}
