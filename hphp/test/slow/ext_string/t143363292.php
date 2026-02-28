<?hh

<<__EntryPoint>> function main(): void {
    for ($i = 0; $i < 23; $i++) {
        $salt = '$2y$04$' . str_repeat('0', $i) . '$';
        $result = crypt("foo", $salt);
        var_dump($salt);
        var_dump($result);
        var_dump($result === $salt);
    }

}
