<?hh

class returnNull implements Countable {
    function count() {
    }
}

class returnString implements Countable {
    function count() {
        return "hello";
    }
}

class returnObject implements Countable {
    function count() {
        return new returnObject;
    }
}

class returnArray implements Countable {
    function count() {
        return varray[1,2,3];
    }
}

class throwException implements Countable {
    function count() {
        throw new Exception('Thrown from count');
    }
}

<<__EntryPoint>> function main(): void {
echo "Count returns null:\n";
var_dump(count(new returnNull));

echo "Count returns a string:\n";
var_dump(count(new returnString));

echo "Count returns an array:\n";
var_dump(count(new returnArray));

echo "Count throws an exception:\n";
try {
    echo count(new throwException);
} catch (Exception $e) {
    echo $e->getMessage();
}
}
