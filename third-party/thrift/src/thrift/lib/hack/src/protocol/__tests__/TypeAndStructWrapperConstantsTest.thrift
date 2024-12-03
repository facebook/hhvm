include "TypeAndStructWrapperTest.thrift"

package "meta.com/thrift/wrapper_test"

namespace hack "WrapperTest"

enum Company {
  // Should not have any impact
  @TypeAndStructWrapperTest.AnnotationStruct
  FACEBOOK = 0,
  WHATSAPP = 1,
  OCULUS = 2,
  INSTAGRAM = 3,
}

typedef Company MyCompany
const MyCompany my_company = FACEBOOK;

struct Internship {
  1: required i32 weeks;
  2: string title;
  3: optional Company employer;
  @TypeAndStructWrapperTest.AnnotationStruct
  4: optional double compensation;
  5: optional string school;
  6: TypeAndStructWrapperTest.i64WithWrapper intern_id;
}

struct SWE {
  1: optional Company employer;
  2: optional double compensation;
}

const Internship instagram = {
  "weeks": 12,
  "title": "Software Engineer",
  "employer": Company.INSTAGRAM,
  "compensation": 1200.0,
  "school": "Monters University",
  "intern_id": 10011,
};

const list<Internship> internList = [
  instagram,
  {
    "weeks": 10,
    "title": "Sales Intern",
    "employer": Company.FACEBOOK,
    "compensation": 1000.0,
    "intern_id": 10013,
  },
];

@TypeAndStructWrapperTest.AnnotationStruct
const list<SWE> engineers = [
  {"employer": Company.FACEBOOK, "compensation": 1000.0},
  {"employer": Company.WHATSAPP, "compensation": 1200.0},
  {"employer": Company.OCULUS, "compensation": 1200.0},
];

const list<TypeAndStructWrapperTest.StructWithWrapper> wrapped_structs = [
  {"int_field": 1},
  {"int_field": 2},
];

const list<TypeAndStructWrapperTest.i64WithWrapper> wrapped_ints = [1, 2];
