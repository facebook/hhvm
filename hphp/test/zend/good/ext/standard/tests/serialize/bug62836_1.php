<?hh

class A {}
class B {}

<<__EntryPoint>> function main(): void {
$serialized = 'O:1:"A":4:{s:1:"b";O:1:"B":0:{}s:2:"b1";r:2;s:1:"c";O:1:"B":0:{}s:2:"c1";r:4;}';
print_r(unserialize($serialized));
echo "okey";
}
