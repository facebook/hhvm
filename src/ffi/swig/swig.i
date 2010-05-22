#define TypeNull 0
#define TypeBoolean 1
#define TypeInt 2
#define TypeDouble 3
#define TypeString 4
#define TypeArray 5
#define TypeObject 6

void hphpStart();

HphpSession *hphpStartSession();

void hphpFinishSession(HphpSession *);

Variant *hphpNull(HphpSession *);

Variant *hphpBoolean(HphpSession *, bool);

Variant *hphpInt(HphpSession *, long long);

Variant *hphpDouble(HphpSession *, double);

Variant *hphpString(HphpSession *, const char *);

Variant *hphpArray(HphpSession *);

int hphpGetType(HphpSession *, Variant *);

bool hphpGetBoolean(HphpSession *, Variant *);

long long hphpGetInt(HphpSession *, Variant *);

double hphpGetDouble(HphpSession *, Variant *);

const char *hphpGetString(HphpSession *, Variant *);

void hphpSet(HphpSession *, Variant *, Variant *, Variant *);

Variant *hphpGet(HphpSession *, Variant *, Variant *);

void hphpAppend(HphpSession *, Variant *, Variant *);

void hphpIncludeFile(HphpSession *, const char *);

Variant *hphpInvoke(HphpSession *, const char *, Variant *);

Variant *hphpInvokeMethod(HphpSession *, Variant *, const char *, Variant *);

Variant *hphpInvokeStaticMethod(HphpSession *, const char *, const char *,
                                Variant *);

Variant *hphpNewObject(HphpSession *, const char *, Variant *);

long long hphpIterBegin(HphpSession *, Variant *);

long long hphpIterAdvance(HphpSession *, Variant *, long long);

bool hphpIterValid(HphpSession *, long long);

Variant *hphpIterGetKey(HphpSession *, Variant *, long long);

Variant *hphpIterGetValue(HphpSession *, Variant *, long long);

int hphpCount(HphpSession *, Variant *);

void hphpUnset(HphpSession *, Variant *, Variant *);

bool hphpIsset(HphpSession *, Variant *, Variant *);

Variant *hphpGetField(HphpSession *, Variant *, const char *);

void hphpSetField(HphpSession *, Variant *, const char *, Variant *);

void hphpUnsetField(HphpSession *, Variant *, const char *);

bool hphpIssetField(HphpSession *, Variant *, const char *);

Variant *hphpGetGlobal(HphpSession *, const char *);

void hphpSetGlobal(HphpSession *, const char *, Variant *);

void hphpDump(HphpSession *, Variant *);
