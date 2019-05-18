# Maintainer: candide guevara marino <cguevaramari@gmail.com>
rawname=libhugetlbfs
pkgname=cg-libhugetlbfs
pkgver=2.20
pkgrel=4
epoch=
pkgdesc="libhugetlbfs from debian source tarball"
arch=('x86_64')
url="https://packages.debian.org/source/unstable/libhugetlbfs"
license=('GPL')
groups=()
depends=('bash>=5' 'perl>=5')
makedepends=(make gcc)
checkdepends=(python2)
optdepends=()
provides=('libhugetlbfs')
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=("https://deb.debian.org/debian/pool/main/libh/libhugetlbfs/libhugetlbfs_2.20.orig.tar.gz")
noextract=()
md5sums=('5a588437dfdbf157438d5be5953cf61d')
validpgpkeys=()

prepare() {
  cd "$rawname-$pkgver"
  sed -r -i 's/^\t\$\(CCBIN\)/\t$(CCBIN) -pie -fPIE -z now -z relro/' Makefile
  find . -iname '*.py' -print0 | xargs -0 sed -r -i 's/^#!(.*)python/#!\1python2/'
}

build() {
  cd "$rawname-$pkgver"
  make all BUILDTYPE=NATIVEONLY
}

check() {
  cd "$rawname-$pkgver"
  # checked by namcap
  # find . -type f -iname '*.so' -print0 | xargs -0 -t -I{} ldd "{}"
  # find obj -type f ! -iname '*.o' ! -iname '*.a' -print0 | xargs -0 -t -I{} ldd "{}"
  make check
}

package() {
  local perl_lib_root="`pacman -Ql perl | grep -E '/lib/.*/site_perl/$' | cut -d' ' -f2`"
  test -d "$perl_lib_root"

  cd "$rawname-$pkgver"
  make install BUILDTYPE=NATIVEONLY DESTDIR="${pkgdir}" PREFIX=/usr LIB32=lib32 LIB64=lib

  cd "${pkgdir}"
  local perl_bad_root="`find -type d -name TLBC`"
  local perl_mov_root="$pkgdir/$perl_lib_root"
  test -d "$perl_bad_root"
  mkdir -p "$perl_mov_root"
  mv "$perl_bad_root" "$perl_mov_root"
}

