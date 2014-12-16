#########################################
# Generic Dependency Installer Script   #
#########################################

# Current Script details
SCRIPT="$(basename $0)"
pushd "$(dirname $0)" >/dev/null
SCRIPT_DIR="$(pwd -P)"
popd >/dev/null
PWD="$(pwd -P)"

# Make sure the script is run from its proper path.
# OR Since we know the path, why don't we do it ourselves ?
if [ $PWD != $SCRIPT_DIR ]
then
    echo "Run the script from the HHVM directory like:"
    echo "cd $SCRIPT_DIR && ./$SCRIPT"
    exit 1
fi

# Identify the Distro, determine the CPUs irrespective of Travis Mode
OS_TYPE=$(uname)
if [ "x$OS_TYPE" = "xLinux" ];then
    DISTRO_NAME=$(head -1 /etc/issue)
    CPUS=$(egrep '(^CPU|processor.*:.*)' /proc/cpuinfo|wc -l)
    FREE_MEM=$(free)
    if grep -i fedora /etc/issue >/dev/null 2>&1; then
        DISTRO=fedora
    elif grep -i ubuntu /etc/issue >/dev/null 2>&1;then
        DISTRO=ubuntu
    else
        DISTRO=unknown
    fi
elif [ "x$OS_TYPE" = "xDarwin" ];then
    DISTRO=osx
    FREE_MEM=$(top -l 1|head -n 10|grep PhysMem)
    DISTRO_NAME="$(sw_vers -productName) $(sw_vers -productVersion)"
    CPUS=$(sysctl -n hw.ncpu)
else
    echo "Linux and Darwin are the only supported Operating systems right now."
    echo "Please submit a PR when this script is enhanced to support"
    echo "other operating systems"
    echo ""
    echo "Project URL: https://github.com/facebook/hhvm"
    echo ""
    echo " - Thank You."
    exit 1
fi

echo "Date/Time      : $(date)"
echo "Current Distro : $DISTRO_NAME"

# For travis
if [ "x${TRAVIS}" != "x" ]; then
  # Collect some stats for use in tuning build later on
  echo $FREE_MEM
  echo "Travis Mode  : YES"
  echo "# CPUs       : ${CPUS}"
  echo ""
else
  echo "Travis Mode  : NO"
  echo "# CPUs       : ${CPUS}"
  echo ""
fi

# Place to save all the binaries/libraries/headers from the ext packages
export CMAKE_PREFIX_PATH=`/bin/pwd`/..

case $DISTRO in
    fedora)
        # install the actual dependencies
        sudo yum groupinstall "Fedora Packager" -y
        sudo yum install -y git wget make autoconf binutils-devel \
            boost-devel bzip2-devel chrpath cmake cyrus-sasl elfutils-libelf-devel  \
            expat-devel fontconfig-devel freetype-devel gcc-c++ gd-devel glibc-devel  \
            glog-devel jemalloc-devel keyutils-libs krb5-devel libaio-devel libcap-devel  \
            libc-client libc-client-devel libcom_err-devel libdwarf-devel libedit-devel \
            libicu-devel libjpeg-turbo libjpeg-turbo-devel libmcrypt-devel libmemcached-devel \
            libpng-devel libselinux-devel libsepol-devel libstdc++-devel libtool \
            libunwind-devel libvpx-devel libX11-devel libXau-devel libxcb-devel libxml2-devel \
            libxml++-devel libXpm-devel mysql-devel ncurses-devel oniguruma-devel openldap-devel \
            openssl-devel pam-devel pcre-devel readline-devel tbb-devel unixODBC-devel \
            uw-imap-devel zlib zlib-devel \
            ImageMagick-devel libxslt-devel &

        # For patched stuff.
        git clone git://github.com/libevent/libevent.git --quiet &
        git clone git://github.com/bagder/curl.git --quiet &
        ;;
    ubuntu)
        # install python-software-properties before trying to add a PPA
        sudo apt-get -y update
        sudo apt-get install -y python-software-properties

        # install apt-fast to speed up later dependency installation
        sudo add-apt-repository -y ppa:apt-fast/stable
        sudo add-apt-repository -y ppa:boost-latest/ppa
        sudo apt-get -y update
        sudo apt-get -y install apt-fast

        # install the actual dependencies
        sudo apt-fast -y update
        sudo apt-fast -y install git-core cmake g++ boost1.55 libmysqlclient-dev \
            libxml2-dev libmcrypt-dev libicu-dev openssl build-essential binutils-dev \
            libcap-dev libgd2-xpm-dev zlib1g-dev libtbb-dev libonig-dev libpcre3-dev \
            wget memcached libreadline-dev libncurses-dev libmemcached-dev libbz2-dev \
            libc-client2007e-dev php5-mcrypt php5-imagick libgoogle-perftools-dev \
            libcloog-ppl0 libelf-dev libdwarf-dev libunwind7-dev subversion \
            autoconf libtool libcurl4-openssl-dev \
            libmagickwand-dev libxslt1-dev &

        git clone git://github.com/libevent/libevent.git --quiet &
        git clone git://github.com/bagder/curl.git --quiet &
        svn checkout http://google-glog.googlecode.com/svn/trunk/ google-glog --quiet &
        wget -nc http://www.canonware.com/download/jemalloc/jemalloc-3.6.0.tar.bz2 --quiet &
        ;;
    osx)
        # Force GCC-LLVM to GCC-4.8
        if [ "$TRAVIS_OS_NAME" = "osx" ] && [ "$CC" = "gcc" ]; then
            export CC=gcc-4.8
            export CXX=g++-4.8
            export CPP=cpp-4.8
            export LD=gcc-4.8
            export HOMEBREW_CC=gcc-4.8
            export HOMEBREW_CXX=g++-4.8
        fi

        brew update >/dev/null
        brew tap mcuadros/homebrew-hhvm

        # install the actual dependencies
        #brew install memcached redis
        brew install binutilsfb curl freetype gd gettext git icu4c imagemagick \
                     imap-uw jemallocfb libdwarf libelf libevent libmemcached \
                     libpng libssh2 libvpx libxslt libzip lz4 mcrypt mysql \
                     mysql-connector-c++ objective-caml oniguruma openssl pcre \
                     re2c sqlite tbb unixodbc

        brew install boost gflags glog --build-from-source --cc=gcc-4.8
        ;;
    *)
        echo "Unknown distribution. Please update packages in this section."
        exit 1
        ;;
esac

# init submodules
git submodule update --init --recursive

# wait until all background processes finished
FAIL=0

for job in `jobs -p`
do
    echo "waiting for background job $job"
    wait $job || let "FAIL+=1"
done

if [ "$FAIL" == "0" ];
then
    echo "all downloads finished"
else
    echo "$FAIL errors while downloading!"
    exit 100
fi

if [[ "x$DISTRO" == "xubuntu" ]];then
    # Leave this install till after the main parallel package install above
    # since it adds a non-12.04 package repo and we don't want to
    # pull EVERYTHING in, just the newer gcc compiler (and toolchain)
    GCC_VER=4.8
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo apt-get -y update
    sudo apt-get -y install gcc-${GCC_VER} g++-${GCC_VER}
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VER} 60 \
                                --slave /usr/bin/g++ g++ /usr/bin/g++-${GCC_VER}
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 40 \
                                --slave /usr/bin/g++ g++ /usr/bin/g++-4.6
    sudo update-alternatives --set gcc /usr/bin/gcc-${GCC_VER}
fi

if [[ "x$OS_TYPE" = "xLinux" ]];then
    # libevent
    cd libevent
    git checkout release-1.4.14b-stable
    cat ../third-party/libevent-1.4.14.fb-changes.diff | patch -p1
    ./autogen.sh
    ./configure --prefix=$CMAKE_PREFIX_PATH
    make -j $CPUS
    make install
    cd ..

    # curl
    cd curl
    ./buildconf
    ./configure --prefix=$CMAKE_PREFIX_PATH
    make -j $CPUS
    make install
    cd ..
fi

if [[ "x$DISTRO" == "xubuntu" ]];then
    # glog
    cd google-glog
    ./configure --prefix=$CMAKE_PREFIX_PATH
    make -j $CPUS
    make install
    cd ..

    # jemaloc
    tar xjvf jemalloc-3.6.0.tar.bz2
    cd jemalloc-3.6.0
    ./configure --prefix=$CMAKE_PREFIX_PATH
    make -j $CPUS
    make install
    cd ..

    # cleanup
    rm -rf google-glog jemalloc-3.6.0.tar.bz2 jemalloc-3.6.0
fi

# cleanup
rm -rf libevent curl

# hphp
if [[ "x$OS_TYPE" = "xLinux" ]];then
    cmake "$@" .
elif [ "x$OS_TYPE" = "xDarwin" ];then
    # hard setup osx deps
    OPT=$(brew --prefix)/opt
    cmake "$@" \
      -DCMAKE_INCLUDE_PATH="/usr/local/include:/usr/include" \
      -DCMAKE_LIBRARY_PATH="/usr/local/lib:/usr/lib" \
      -DBOOST_INCLUDEDIR=$OPT/boost/include -DBOOST_LIBRARYDIR=$OPT/boost/lib -DBoost_USE_STATIC_LIBS=ON \
      -DCCLIENT_INCLUDE_PATH=$OPT/imap-uw/include/imap \
      -DCURL_INCLUDE_DIR=$OPT/curl/include -DCURL_LIBRARY=$OPT/curl/lib/libcurl.dylib \
      -DFREETYPE_INCLUDE_DIRS=$OPT/freetype/include/freetype2 -DFREETYPE_LIBRARIES=$OPT/freetype/lib/libfreetype.dylib \
      -DICU_DATA_LIBRARY=$OPT/icu4c/lib/libicudata.dylib -DICU_I18N_LIBRARY=$OPT/icu4c/lib/libicui18n.dylib -DICU_INCLUDE_DIR=$OPT/icu4c/include -DICU_LIBRARY=$OPT/icu4c/lib/libicuuc.dylib \
      -DJEMALLOC_INCLUDE_DIR=$OPT/jemallocfb/include -DJEMALLOC_LIB=$OPT/jemallocfb/lib/libjemalloc.dylib \
      -DLBER_LIBRARIES=/usr/lib/liblber.dylib \
      -DLDAP_INCLUDE_DIR=/usr/include -DLDAP_LIBRARIES=/usr/lib/libldap.dylib \
      -DLIBDWARF_INCLUDE_DIRS=$OPT/libdwarf/include -DLIBDWARF_LIBRARIES=$OPT/libdwarf/lib/libdwarf.3.dylib \
      -DLIBELF_INCLUDE_DIRS=$OPT/libelf/include/libelf \
      -DLIBEVENT_INCLUDE_DIR=$OPT/libevent/include -DLIBEVENT_LIB=$OPT/libevent/lib/libevent.dylib \
      -DLIBGLOG_INCLUDE_DIR=$OPT/glog/include \
      -DLIBINTL_INCLUDE_DIR=$OPT/gettext/include -DLIBINTL_LIBRARIES=$OPT/gettext/lib/libintl.dylib \
      -DLIBJPEG_INCLUDE_DIRS=$OPT/jpeg/include \
      -DLIBMAGICKWAND_INCLUDE_DIRS=$OPT/imagemagick/include/ImageMagick-6 -DLIBMAGICKWAND_LIBRARIES=$OPT/imagemagick/lib/libMagickWand-6.Q16.dylib \
      -DLIBMEMCACHED_INCLUDE_DIR=$OPT/libmemcached/include \
      -DLIBODBC_INCLUDE_DIRS=$OPT/unixodbc/include \
      -DLIBPNG_INCLUDE_DIRS=$OPT/libpng/include \
      -DLIBSQLITE3_INCLUDE_DIR=$OPT/sqlite/include -DLIBSQLITE3_LIBRARY=$OPT/sqlite/lib/libsqlite3.0.dylib \
      -DLIBVPX_INCLUDE_DIRS=$OPT/libvpx/include -DLIBVPX_LIBRARIES=$OPT/libvpx/lib/libvpx.a \
      -DLIBZIP_INCLUDE_DIR_ZIP=$OPT/libzip/include -DLIBZIP_INCLUDE_DIR_ZIPCONF=$OPT/libzip/lib/libzip/include -DLIBZIP_LIBRARY=$OPT/libzip/lib/libzip.dylib \
      -DLZ4_INCLUDE_DIR=$OPT/lz4/include -DLZ4_LIBRARY=$OPT/lz4/lib/liblz4.dylib \
      -DMcrypt_INCLUDE_DIR=$OPT/mcrypt/include \
      -DOCAMLC_EXECUTABLE=$OPT/objective-caml/bin/ocamlc -DOCAMLC_OPT_EXECUTABLE=$OPT/objective-caml/bin/ocamlc.opt \
      -DONIGURUMA_INCLUDE_DIR=$OPT/oniguruma/include \
      -DPCRE_INCLUDE_DIR=$OPT/pcre/include -DPCRE_LIBRARY=$OPT/pcre/lib/libpcre.dylib -DSYSTEM_PCRE_INCLUDE_DIR=$OPT/pcre/include -DSYSTEM_PCRE_LIBRARY=$OPT/pcre/lib/libpcre.dylib \
      -DREADLINE_INCLUDE_DIR=$OPT/readline/include -DREADLINE_LIBRARY=$OPT/readline/lib/libreadline.dylib \
      -DTBB_INCLUDE_DIRS=$OPT/tbb/include -DTEST_TBB_INCLUDE_DIR=$OPT/tbb/include \
      -DCMAKE_INCLUDE_PATH=$OPT/binutilsfb/include -DLIBIBERTY_LIB=$OPT/binutilsfb/lib/x86_64/libiberty.a -DBFD_LIB=$OPT/binutilsfb/lib/libbfd.a \
      -DCMAKE_BUILD_TYPE=MinSizeRel \
      -DMYSQL_INCLUDE_DIR=$OPT/mysql/include/mysql -DMYSQL_LIB_DIR=$OPT/mysql/lib \
      -DOPENSSL_SSL_LIBRARY=$OPT/openssl/lib/libssl.dylib -DOPENSSL_INCLUDE_DIR=$OPT/openssl/include -DOPENSSL_CRYPTO_LIBRARY=$OPT/openssl/lib/libcrypto.dylib \
      .
fi

# all set
echo "-------------------------------------------------------------------------"
echo "Done. Now run:"
echo "  CMAKE_PREFIX_PATH=\`pwd\`/.. make"
