#autoscan
set -x
cd ${1-=`pwd`}
aclocal
autoconf
autoheader
automake --add-missing
./configure
make
