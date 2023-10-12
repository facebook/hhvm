<?hh

abstract ckass C { // error1058
}

final interfsce I { // error1058
}

abstract final ttait T { // error1058
}

// Error recovery in this case remains unsophisticated, though.

ckass C {
}
