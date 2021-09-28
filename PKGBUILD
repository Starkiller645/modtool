# This is an example PKGBUILD file. Use this as a start to creating your own,
# and remove these comments. For more information, see 'man PKGBUILD'.
# NOTE: Please fill out the license field for your package! If it is unknown,
# then please put 'unknown'.

# Maintainer: Your Name <jacob.tye@outlook.com>
pkgname="modtool"
pkgver=1.0.1
pkgrel=1
arch=("x86_64" "arm")
pkgdesc="Mod Tool - for installing mods for the server"
url="https://git.jacobtye.dev/Starkiller645/modtool/"
license=('LGPL')
depends=('qt5-base' 'qt5-networkauth' 'openssl')
makedepends=('cmake' 'gcc')
install=
source=("https://git.jacobtye.dev/Starkiller645/$pkgname/archive/v$pkgver.tar.gz")
noextract=()
sha256sums=("390d3ab3c56b3e1236bdcc0b612400cf85eaec024ac06d1bea418e535c7cfb71")
build() {
	cd "$pkgname"
	mkdir build
	cd build
	cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
	make
}

package() {
	cd "$pkgname/build"
	make DESTDIR="$pkgdir" PREFIX="/usr" install
}

