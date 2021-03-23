<?hh // strict

class Ec implements HH\EnumClassAttribute {}
class Ec2 implements HH\EnumClassAttribute {}
class Ec3 implements HH\EnumClassAttribute {}

<<Ec, Ec2, Ec3>>
enum class Ecc : int {}
