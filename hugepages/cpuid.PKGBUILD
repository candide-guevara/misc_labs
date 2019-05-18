# Maintainer: candide guevara marino <cguevaramari@gmail.com>
rawname=cpuid
pkgname=cg-cpuid
pkgver=20180519
pkgrel=1
epoch=
pkgdesc="cpuid from debian source tarball"
arch=('x86_64')
url="https://packages.debian.org/source/unstable/cpuid"
license=('GPL')
groups=()
depends=('perl>=5')
makedepends=(make gcc gzip 'perl>=5')
checkdepends=()
optdepends=()
provides=(libcpuid)
conflicts=(libcpuid)
replaces=()
backup=()
options=()
install=
changelog=
source=("https://deb.debian.org/debian/pool/main/c/cpuid/cpuid_20180519.orig.tar.gz" )
noextract=()
md5sums=('b3b4e44ef49575043a91def0207dcc76')
validpgpkeys=()

prepare() {
  true
}

build() {
  cd "$rawname-$pkgver"
  sed -r -i 's/^[[:space:]]*char[[:space:]]+([[:alnum:]_]+)\[48\];/char \1[4096];/' cpuid.c
  make default CFLAGS='-O2 -fPIE -pie -z now -z relro'
}

check() {
  true
}

package() {
  cd "$rawname-$pkgver"
  make install BUILDROOT="${pkgdir}"

  cd "${pkgdir}"
  local -a perl_bins=( `grep -REl '#!.*perl' .` )
  test -d "/usr/bin/site_perl"
  local perl_dst="$pkgdir/usr/bin/site_perl"
  mkdir -p "$perl_dst"
  mv "${perl_bins[@]}" "$perl_dst"
}
