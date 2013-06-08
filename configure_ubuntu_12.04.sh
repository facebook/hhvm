#########################################
# 
# Install all the dependancies for HipHop
#
#########################################

SCRIPT_NAME='./configure_ubuntu_12.04.sh'
if [ "$0" != "$SCRIPT_NAME" ]; then
  echo "Run the script from the hiphop-php directory like:"
  echo "  $SCRIPT_NAME"
  exit 1
fi

export CMAKE_PREFIX_PATH=`/bin/pwd`/..
export HPHP_HOME=`/bin/pwd`

sudo apt-get install git-core cmake g++ libboost1.48-dev libmysqlclient-dev \
  libxml2-dev libmcrypt-dev libicu-dev openssl build-essential binutils-dev \
  libcap-dev libgd2-xpm-dev zlib1g-dev libtbb-dev libonig-dev libpcre3-dev \
  autoconf libtool libcurl4-openssl-dev libboost-regex1.48-dev libboost-system1.48-dev \
  libboost-program-options1.48-dev libboost-filesystem1.48-dev wget memcached \
  libreadline-dev libncurses-dev libmemcached-dev libbz2-dev \
  libc-client2007e-dev php5-mcrypt php5-imagick libgoogle-perftools-dev \
  libcloog-ppl0 libelf-dev libdwarf-dev libunwind7-dev subversion

# libevent
git clone git://github.com/libevent/libevent.git
cd libevent
git checkout release-1.4.14b-stable
cat ../hphp/third_party/libevent-1.4.14.fb-changes.diff | patch -p1
./autogen.sh
./configure --prefix=$CMAKE_PREFIX_PATH
make
make install
cd ..

# curl
git clone git://github.com/bagder/curl.git
cd curl
./buildconf
./configure --prefix=$CMAKE_PREFIX_PATH
make
make install
cd ..

# glog
svn checkout http://google-glog.googlecode.com/svn/trunk/ google-glog
cd google-glog
./configure --prefix=$CMAKE_PREFIX_PATH
make
make install
cd ..

# jemaloc
wget http://www.canonware.com/download/jemalloc/jemalloc-3.0.0.tar.bz2
tar xjvf jemalloc-3.0.0.tar.bz2
cd jemalloc-3.0.0
./configure --prefix=$CMAKE_PREFIX_PATH
make
make install
cd ..

# cleanup
rm -rf libevent curl google-glog jemalloc-3.0.0.tar.bz2 jemalloc-3.0.0

# hphp
cmake .

echo "-------------------------------------------------------------------------"
echo "Done. Now run:"
echo "  CMAKE_PREFIX_PATH=\`pwd\`/.. HPHP_HOME=\`pwd\` make"
