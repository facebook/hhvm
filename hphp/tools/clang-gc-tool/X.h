#ifndef incl_X_H
#define incl_X_H
namespace HPHP {
class ResourceData { };
class ObjectData { };
class Variant { };
class Object { };
}

namespace std {
template <typename T> struct vector { T* data; };
}

namespace req {
template <typename T> struct vector { T* data; };
}

class foobar : HPHP::ResourceData {
  int scanme;
};

// empty class, should use parent's scan
class foobar2 : public foobar { };

struct bad_class {
  foobar ff;
};

struct bad_class2 {
  std::vector<foobar> vec;
};

struct not_bad_class {
  req::vector<foobar> vec;
};

struct bad_template_use : public HPHP::ResourceData {
  std::vector<foobar> vec;
};

class X;
/** HHVM_NEEDS_SCAN */
class X : public HPHP::ResourceData { int v; char* s; public: X* x; };

typedef union { int i; double d; } U;

namespace Y {
/** HHVM_NEEDS_SCAN */
struct Z : public X {
  int y;
  /** HHVM_USER_SCAN_METHOD scanM(y, m) */
  union { int i; double d; } m;
  /** HHVM_USER_SCAN_METHOD scanU(u) */
  U u;
};
}

/** no mark fn */
class W { };

/** HHVM_NEEDS_SCAN but won't get one */
union Q { };

struct voidstar {
  const void* p;
};

struct voidstargc {
  HPHP::ResourceData rd;
  const void* p;
};

namespace ns {
class privateX;
}

struct opaqueRef {
  ns::privateX* px;
};

struct badcontainer : public HPHP::ResourceData {
  std::vector<HPHP::ResourceData> vec;
};

template <typename T>
class hide_template {
  T* x;
};

struct hide_helper {
  hide_template<HPHP::ResourceData> field;
};

struct indirect : public HPHP::ResourceData {
  struct {
    HPHP::ResourceData a;
    HPHP::ResourceData b;
  };
  HPHP::Variant v;
};
#endif
