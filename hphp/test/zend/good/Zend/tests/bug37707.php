<?hh
class testme {
    function __clone() :mixed{
        echo "clonned\n";
    }
}
<<__EntryPoint>> function main(): void {
clone new testme();
echo "NO LEAK\n";
}
