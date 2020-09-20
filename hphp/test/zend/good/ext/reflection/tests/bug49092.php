<?hh
namespace ns;
function func(){}
<<__EntryPoint>> function main(): void {
new \ReflectionFunction('ns\func');
new \ReflectionFunction('\ns\func');
echo "Ok\n";
}
