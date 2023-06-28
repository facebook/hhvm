<?hh
function F () :mixed{
    if(1) {
        return("Hello");
    }
}
<<__EntryPoint>> function main(): void {
$i=0;
while ($i<2) {
    echo F();
    $i++;
}
}
