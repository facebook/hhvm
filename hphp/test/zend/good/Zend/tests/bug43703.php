<?hh
class JoinPoint
{
}

abstract class Pointcut
{
    abstract public function evaluate(JoinPoint $joinPoint):mixed;
}

class Read extends Pointcut
{
    public function evaluate(JoinPoint $joinPoint)
:mixed    {
    }
}
<<__EntryPoint>> function main(): void {
echo "DONE";
}
