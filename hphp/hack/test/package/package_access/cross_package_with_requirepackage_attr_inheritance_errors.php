<?hh
// package pkg1

// Nomenclature: [Parent]_[child] method annotation and 2 is the child

class Req_req {
  <<__RequirePackage("pkg3")>>
  public function ok(): void {}
  <<__RequirePackage("pkg2")>>
  public function err(): void {}
  <<__RequirePackage("pkg2")>>
  public function soft(): void {}
}
class Req_req2 extends Req_req {
  <<__Override, __RequirePackage("pkg2")>>
  public function ok(): void {}
  <<__Override, __RequirePackage("pkg3")>>
  public function err(): void {}
  <<__Override, __RequirePackage("pkg2_soft")>>
  public function soft(): void {}
}

class Soft_req {
  <<__SoftRequirePackage("pkg2")>>
  public function err(): void {}
}
class Soft_req2 extends Soft_req {
  <<__Override, __RequirePackage("pkg2")>>
  public function err(): void {}
}

class Normal_req {
  public function err(): void {}
}
class Normal_req2 extends Normal_req {
  <<__Override, __RequirePackage("pkg2")>>
  public function err(): void {}
}

class Req_soft {
  <<__RequirePackage("pkg3")>>
  public function ok(): void {}
  <<__RequirePackage("pkg2")>>
  public function err(): void {}
  <<__RequirePackage("pkg2")>>
  public function soft(): void {}
}
class Req_soft2 extends Req_soft {
  <<__Override, __SoftRequirePackage("pkg2")>>
  public function ok(): void {}
  <<__Override, __SoftRequirePackage("pkg3")>>
  public function err(): void {}
  <<__Override, __SoftRequirePackage("pkg2_soft")>>
  public function soft(): void {}
}

class Soft_soft {
  <<__SoftRequirePackage("pkg3")>>
  public function ok(): void {}
  <<__SoftRequirePackage("pkg2")>>
  public function err(): void {}
  <<__SoftRequirePackage("pkg2")>>
  public function soft(): void {}
}
class Soft_soft2 extends Soft_soft {
  <<__Override, __SoftRequirePackage("pkg2")>>
  public function ok(): void {}
  <<__Override, __SoftRequirePackage("pkg3")>>
  public function err(): void {}
  <<__Override, __SoftRequirePackage("pkg2_soft")>>
  public function soft(): void {}
}

class Normal_soft {
  public function err(): void {}
}
class Normal_soft2 extends Normal_soft {
  <<__Override, __SoftRequirePackage("pkg2")>>
  public function err(): void {}
}

class Soft_normal {
  <<__SoftRequirePackage("pkg3")>>
  public function ok(): void {}
}
class Soft_normal2 extends Soft_normal {
  <<__Override>>
  public function ok(): void {}
}

class Req_normal {
  <<__RequirePackage("pkg3")>>
  public function ok(): void {}
}
class Req_normal2 extends Req_normal {
  <<__Override>>
  public function ok(): void {}
}

class Normal_normal {
  public function ok(): void {}
}
class Normal_normal2 extends Normal_normal {
  <<__Override>>
  public function ok(): void {}
}
