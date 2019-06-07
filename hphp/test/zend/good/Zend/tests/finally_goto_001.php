<?hh
function foo() {
	goto test;
	try {
    } finally {
test:
    }
}
