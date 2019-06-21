<?hh
function foo() {
	try {
    } finally {
	goto test;
test:
    }
}

<<__EntryPoint>> function main(): void {
echo "okey";
}
