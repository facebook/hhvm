---
title: ErrnoException
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Base class for exceptions reported via the C ` errno ` variable




Subclasses exist for some specific ` Errno ` values, such as:

+ ` ChildProcessException ` (`` ECHILD ``)
+ ` ConnectionException ` and its' subclasses, `` BrokenPipeException ``
  (``` EPIPE ```, ```` ESHUTDOWN ````), ````` ConnectionAbortedException ````` (`````` ECONNABORTED ``````),
  ``````` ConnectionRefusedException ``````` (```````` ECONNREFUSED ````````), and
  ````````` ConnectionResetException ````````` (`````````` ECONNRESET ``````````)
+ ` AlreadyExistsException ` (`` EEXIST ``)
+ ` NotFoundException ` (`` ENOENT ``)
+ ` IsADirectoryException ` (`` EISDIR ``)
+ ` IsNotADirectoryException ` (`` ENOTDIR ``)
+ ` PermissionException ` (`` EACCESS ``, ``` EPERM ```)
+ ` ProcessLookupException ` (`` ESRCH ``)
+ ` TimeoutError ` (`` ETIMEDOUT ``)




It is strongly recommended to catch subclasses instead of this class if a
suitable subclass is defined; for example:




``` Hack
// ANTIPATTERN:
catch (OS\ErrnoException $e) {
  if ($e->getErrno() === OS\Errno::ENOENT) {
    do_stuff();
  }
}
// RECOMMENDED:
catch (OS\NotFoundException $_) {
  do_stuff();
}
```




If a suitable subclass is not defined, the antipattern is unavoidable.




## Interface Synopsis




``` Hack
namespace HH\Lib\OS;

class ErrnoException extends \Exception {...}
```




### Public Methods




* [` ->__construct(Errno $errno, string $message) `](/hsl/Classes/HH.Lib.OS/ErrnoException/__construct/)
* [` ->getCode(): Errno `](/hsl/Classes/HH.Lib.OS/ErrnoException/getCode/)\
  Deprecated for clarity, and potential future ambiguity
* [` ->getErrno(): Errno `](/hsl/Classes/HH.Lib.OS/ErrnoException/getErrno/)
<!-- HHAPIDOC -->
