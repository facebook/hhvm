<?hh
newtype BaseIGID<T1, T2> as int = int;
newtype BaseFBID<T1, T2, T3> as int = int;
class IMETAEntInstagramUser {}
class InIGIDRange {}
class IN_FBID_RANGE {}
class FBID_FORBIDS_ZERO {}
class IGEntMedia {}
class IN_OID_RANGE {}
<<__PHPStdLib>>
function make_igid(): BaseIGID<IMETAEntInstagramUser, InIGIDRange>;
<<__PHPStdLib>>
function make_fbid(): BaseFBID<IMETAEntInstagramUser, IN_FBID_RANGE, FBID_FORBIDS_ZERO>;
<<__PHPStdLib>>
function make_media_id(): BaseFBID<IGEntMedia, IN_OID_RANGE, FBID_FORBIDS_ZERO>;
