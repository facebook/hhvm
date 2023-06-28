<?hh
namespace ns;
<<__DynamicallyCallable>>
function func():mixed{}
<<__EntryPoint>> function main(): void {
new \ReflectionFunction('ns\func');
new \ReflectionFunction('\ns\func');
echo "Ok\n";
}
