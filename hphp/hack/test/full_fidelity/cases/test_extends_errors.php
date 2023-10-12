<?hh

// legal
interface c extends a, b {
}

// error
class c extends a, b {
}

// error
trait c extends a, b {
}

// legal
interface c extends a {
}

// legal
class c extends a {
}

// error
trait c extends a {
}

// error 1007
interface c extends {
}

// error 1007
class c extends {
}

// errors 1007 and 2036
trait c extends {
}
