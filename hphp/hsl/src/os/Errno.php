<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\OS;

use namespace HH\Lib\_Private\_OS;

// hackfmt-ignore
/** OS-level error number constants from `errno.h`.
 *
 * These values are typically stored in a global `errno` variable by C APIs.
 *
 * **It is unsafe to call `Errno::getNames()` as this enum contains duplicate
 * values**; this is unavoidable for portability:
 * - on MacOS, `EOPNOTSUPP` and `ENOTSUP` are distinct values, so must be
 *   represented separately in this enum
 *  - On Linux, they are equal, so are a duplicate value.
 *
 * `0` is used to indicate success, but not defined in `errno.h`; we expect
 * Hack programs to use the `Errno` type when an error is known to have
 * occurred, or `?Errno` when an error /may/ have occurred.
 *
 * Negative values indicate that the constant is not defined on the current
 * operating system; for example, `ECHRNG` is not defined on MacOS.
 *
 * Constants are defined in this namespace by the runtime, but currently only
 * if they are defined on all supported platforms; in this enum we manually
 * specify the non-portable ones for now.
 */
enum Errno: int as int {
  /* SUCCESS = 0 */
  EPERM           = _OS\EPERM;
  ENOENT          = _OS\ENOENT;
  ESRCH           = _OS\ESRCH;
  EINTR           = _OS\EINTR;
  EIO             = _OS\EIO;
  ENXIO           = _OS\ENXIO;
  E2BIG           = _OS\E2BIG;
  ENOEXEC         = _OS\ENOEXEC;
  EBADF           = _OS\EBADF;
  ECHILD          = _OS\ECHILD;
  EAGAIN          = _OS\EAGAIN;
  ENOMEM          = _OS\ENOMEM;
  EACCES          = _OS\EACCES;
  EFAULT          = _OS\EFAULT;
  ENOTBLK         = _OS\ENOTBLK;
  EBUSY           = _OS\EBUSY;
  EEXIST          = _OS\EEXIST;
  EXDEV           = _OS\EXDEV;
  ENODEV          = _OS\ENODEV;
  ENOTDIR         = _OS\ENOTDIR;
  EISDIR          = _OS\EISDIR;
  EINVAL          = _OS\EINVAL;
  ENFILE          = _OS\ENFILE;
  EMFILE          = _OS\EMFILE;
  ENOTTY          = _OS\ENOTTY;
  ETXTBSY         = _OS\ETXTBSY;
  EFBIG           = _OS\EFBIG;
  ENOSPC          = _OS\ENOSPC;
  ESPIPE          = _OS\ESPIPE;
  EROFS           = _OS\EROFS;
  EMLINK          = _OS\EMLINK;
  EPIPE           = _OS\EPIPE;
  EDOM            = _OS\EDOM;
  ERANGE          = _OS\ERANGE;
  EDEADLK         = _OS\EDEADLK;
  ENAMETOOLONG    = _OS\ENAMETOOLONG;
  ENOLCK          = _OS\ENOLCK;
  ENOSYS          = _OS\ENOSYS;
  ENOTEMPTY       = _OS\ENOTEMPTY;
  ELOOP           = _OS\ELOOP;
  EWOULDBLOCK     = _OS\EAGAIN; // alias
  ENOMSG          = _OS\ENOMSG;
  EIDRM           = _OS\EIDRM;

  ECHRNG          = _OS\IS_MACOS ?  -44 :   44;
  EL2NSYNC        = _OS\IS_MACOS ?  -45 :   45;
  EL3HLT          = _OS\IS_MACOS ?  -46 :   46;
  EL3RST          = _OS\IS_MACOS ?  -47 :   47;
  ELNRNG          = _OS\IS_MACOS ?  -48 :   48;
  EUNATCH         = _OS\IS_MACOS ?  -49 :   49;
  ENOCSI          = _OS\IS_MACOS ?  -50 :   50;
  EL2HLT          = _OS\IS_MACOS ?  -51 :   51;
  EBADE           = _OS\IS_MACOS ?  -52 :   52;
  EBADR           = _OS\IS_MACOS ?  -53 :   53;
  EXFULL          = _OS\IS_MACOS ?  -54 :   54;
  ENOANO          = _OS\IS_MACOS ?  -55 :   55;
  EBADRQC         = _OS\IS_MACOS ?  -56 :   56;
  EBADSLT         = _OS\IS_MACOS ?  -57 :   57;
  EDEADLOCK       = _OS\EDEADLK;

  EBFONT          = _OS\IS_MACOS ?  -59 :   59;
  ENOSTR          = _OS\ENOSTR;
  ENODATA         = _OS\ENODATA;
  ETIME           = _OS\ETIME;
  ENOSR           = _OS\ENOSR;
  ENONET          = _OS\IS_MACOS ?  -64 :   64;
  ENOPKG          = _OS\IS_MACOS ?  -65 :   65;
  EREMOTE         = _OS\IS_MACOS ?  -66 :   66;
  ENOLINK         = _OS\ENOLINK;
  EADV            = _OS\IS_MACOS ?  -68 :   68;
  ESRMNT          = _OS\IS_MACOS ?  -69 :   69;
  ECOMM           = _OS\IS_MACOS ?  -70 :   70;
  EPROTO          = _OS\EPROTO;
  EMULTIHOP       = _OS\EMULTIHOP;
  EDOTDOT         = _OS\IS_MACOS ?  -73 :   73;
  EBADMSG         = _OS\EBADMSG;
  EOVERFLOW       = _OS\EOVERFLOW;
  ENOTUNIQ        = _OS\IS_MACOS ?  -76 :   76;
  EBADFD          = _OS\IS_MACOS ?  -77 :   77;
  EREMCHG         = _OS\IS_MACOS ?  -78 :   78;

  ELIBACC         = _OS\IS_MACOS ?  -79 :   79;
  ELIBBAD         = _OS\IS_MACOS ?  -80 :   80;
  ELIBSCN         = _OS\IS_MACOS ?  -81 :   81;
  ELIBMAX         = _OS\IS_MACOS ?  -82 :   82;
  ELIBEXEC        = _OS\IS_MACOS ?  -83 :   83;

  EILSEQ          = _OS\EILSEQ;
  ERESTART        = _OS\IS_MACOS ?  -85 :   85;
  ESTRPIPE        = _OS\IS_MACOS ?  -86 :   86;
  EUSERS          = _OS\EUSERS;
  ENOTSOCK        = _OS\ENOTSOCK;
  EDESTADDRREQ    = _OS\EDESTADDRREQ;
  EMSGSIZE        = _OS\EMSGSIZE;
  EPROTOTYPE      = _OS\EPROTOTYPE;
  ENOPROTOOPT     = _OS\ENOPROTOOPT;
  EPROTONOSUPPORT = _OS\EPROTONOSUPPORT;
  ESOCKTNOSUPPORT = _OS\ESOCKTNOSUPPORT;
  ENOTSUP         = _OS\ENOTSUP;
  EOPNOTSUPP      = _OS\EOPNOTSUPP;
  EPFNOSUPPORT    = _OS\EPFNOSUPPORT;
  EAFNOSUPPORT    = _OS\EAFNOSUPPORT;
  EADDRINUSE      = _OS\EADDRINUSE;
  EADDRNOTAVAIL   = _OS\EADDRNOTAVAIL;
  ENETDOWN        = _OS\ENETDOWN;
  ENETUNREACH     = _OS\ENETUNREACH;
  ENETRESET       = _OS\ENETRESET;
  ECONNABORTED    = _OS\ECONNABORTED;
  ECONNRESET      = _OS\ECONNRESET;
  ENOBUFS         = _OS\ENOBUFS;
  EISCONN         = _OS\EISCONN;
  ENOTCONN        = _OS\ENOTCONN;
  ESHUTDOWN       = _OS\ESHUTDOWN;
  ETOOMANYREFS    = _OS\IS_MACOS ? -109 :  109;
  ETIMEDOUT       = _OS\ETIMEDOUT;
  ECONNREFUSED    = _OS\ECONNREFUSED;
  // MacOS:
  // 62: ELOOP (35)
  // 63: ENAMETOOLONG (36)
  EHOSTDOWN       = _OS\EHOSTDOWN;
  EHOSTUNREACH    = _OS\EHOSTUNREACH;
  // 66: ENOTEMPTY (39)
  EPROCLIM        = _OS\IS_MACOS ?   67 :  -67;
  // 68: EUSERS (87)
  // 69: EDQUOT (112)
  EALREADY        = _OS\EALREADY;
  EINPROGRESS     = _OS\EINPROGRESS;
  ESTALE          = _OS\ESTALE;

  EUCLEAN         = _OS\IS_MACOS ? -117 :  117;
  ENOTNAM         = _OS\IS_MACOS ? -118 :  118;
  ENAVAIL         = _OS\IS_MACOS ? -119 :  119;
  EISNAM          = _OS\IS_MACOS ? -120 :  120;
  EREMOTEIO       = _OS\IS_MACOS ? -121 :  121;
  EDQUOT          = _OS\EDQUOT;

  ENOMEDIUM       = _OS\IS_MACOS ? -123 :  123;
  EMEDIUMTYPE     = _OS\IS_MACOS ? -124 :  124;

  // MacOS Extensions
  EBADRPC         = _OS\IS_MACOS ?   72 :  -72;
  ERPCMISMATCH    = _OS\IS_MACOS ?   73 :  -73;
  EPROGUNAVAIL    = _OS\IS_MACOS ?   74 :  -74;
  EPROGMISMATCH   = _OS\IS_MACOS ?   75 :  -75;
  EPROCUNAVAIL    = _OS\IS_MACOS ?   76 :  -76;
  // 77: ENOLCK (37)
  // 78: ENOSYS (38)
  EFTYPE          = _OS\IS_MACOS ?   79 :  -79;
  EAUTH           = _OS\IS_MACOS ?   80 :  -80;
  ENEEDAUTH       = _OS\IS_MACOS ?   81 :  -81;
  EPWROFF         = _OS\IS_MACOS ?   82 :  -82;
  EDEVERR         = _OS\IS_MACOS ?   83 :  -83;
  // 84: EOVERFLOW (75)
  EBADARCH        = _OS\IS_MACOS ?   86 :  -86;
  ESHLIBVERS      = _OS\IS_MACOS ?   87 :  -87;
  EBADMACHO       = _OS\IS_MACOS ?   88 :  -88;
  ECANCELLED      = _OS\IS_MACOS ?   89 :  -89;
  // 90: EIDRM (43)
  // 91: ENOMSG (42)
  // 92: EILSEQ (84)
  ENOATTR         = _OS\IS_MACOS ?   93 :  -93;
}
