<?php
class JoinPoint
{
}

abstract class Pointcut
{
    abstract public function evaluate(JoinPoint $joinPoint);
}

class Read extends Pointcut
{
    public function evaluate(Joinpoint $joinPoint)
    {
    }
}
?>
DONE