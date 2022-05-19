{ fetchurl
, lib
, libelf
, stdenv
, zlib
}:
stdenv.mkDerivation rec {
  pname = "libdwarf";
  version = "20210528";

  src = fetchurl {
    url = "https://www.prevanders.net/libdwarf-${version}.tar.gz";
    # Upstream displays this hash broken into four parts:
    sha512 =
      "e0f9c88554053ee6c1b1333960891189"
      + "e7820c4a4ddc302b7e63754a4cdcfc2a"
      + "cb1b4b6083a722d1204a75e994fff340"
      + "1ecc251b8c3b24090f8cb4046d90f870";
  };

  configureFlags = [ "--enable-shared" "--disable-nonshared" ];

  buildInputs = [ libelf zlib ];

  meta = {
    homepage = "https://www.prevanders.net/dwarf.html";
    platforms = lib.platforms.unix;
    license = lib.licenses.lgpl21Plus;
  };
}
