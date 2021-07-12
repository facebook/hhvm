<?hh <<__EntryPoint>> function main(): void {
$h = new SplMaxHeap();
echo "Checking a new heap is empty: ";
(string)(var_dump($h->isEmpty()))."\n";
$h->insert(2);
echo "Checking after insert: ";
(string)(var_dump($h->isEmpty()))."\n";
$h->extract();
echo "Checking after extract: ";
(string)(var_dump($h->isEmpty()))."\n";
}
