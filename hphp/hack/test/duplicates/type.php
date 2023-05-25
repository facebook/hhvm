//// a.php
<?hh
class mcci {} // "m"ultifile, "c"lass in a.php, "c"lass in b.php, "i"nsensitive (i.e. differ in case)
class mcc {} // as above, but they don't differ in case
class mcti {} // "c"lass in a.php vs "t"ypedef in b.php
class mct {}
type mtti = int;
type mtt = int;
type mtci = int;
type mtc = int;

class cci {} // similar to the above, but not multifile; all just in a single file.
class CCI {}
class cc {}
class cc {}
class cti {}
type CTI = string;
class ct {}
type ct = string;
type tti = int;
type TTI = string;
type tt = int;
type tt = string;
type tci = int;
class TCI {}
type tc = int;
class tc {}

class c {} // here are some non-dupe definitions!
type t = int;

//// b.php
<?hh
class MCCI {}
class mcc {}
type MCTI = string;
type mct = string;
type MTTI = string;
type mtt = string;
class MTCI {}
class mtc {}
