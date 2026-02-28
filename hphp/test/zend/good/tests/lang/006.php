<?hh <<__EntryPoint>> function main(): void {
$a=1;
$b=2;

if($a==0) {
    echo "bad";
} else if($a==3) {
    echo "bad";
} else {
    if($b==1) {
        echo "bad";
    } else if($b==2) {
        echo "good";
    } else {
        echo "bad";
    }
}
}
