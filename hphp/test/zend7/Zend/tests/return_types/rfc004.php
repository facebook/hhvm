<?php

class User {}

interface UserGateway {
    function find($id) : User;
}

class UserGateway_MySql implements UserGateway {
    // must return User or subtype of User
    function find($id) {
        return new User;
    }
}

