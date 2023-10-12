<?hh

interface I1 { // legal
}

abstract interface I1 { // errror2042
}

class C1 { // legal
}

abstract class C2 { // legal
}

trait T { // legal
}

abstract trait T { // error2043
}
