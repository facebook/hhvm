<?hh

function fn($reqParam, $optParam = null, ...$params) :mixed{
    var_dump($reqParam, $optParam, $params);
}
<<__EntryPoint>> function main(): void {
fn(1);
fn(1, 2);
fn(1, 2, 3);
fn(1, 2, 3, 4);
fn(1, 2, 3, 4, 5);
}
