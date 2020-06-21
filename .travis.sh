#/bin/bash

export USE_CCACHE=1
export HOMEBREW_NO_INSTALL_CLEANUP=1
export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_ANALYTICS=1
export ROOT_DIR=$(pwd)
export NINJA_STATUS_SLEEP=2000

travis_before_install() {
    # Here should go changes needed in the repo or system before continuing e.g git submodules
    # ESBMC does not have any for now.
    echo "Configuring repository"

    if [ "$TRAVIS_OS_NAME" = osx ]; then
        # https://docs.travis-ci.com/user/caching/#ccache-on-macos
        export PATH="/usr/local/opt/ccache/libexec:$PATH"
    fi

    pip3 install toml
}

travis_install() {
    # Here are dependencies that were not installed by the addons e.g solvers, llvm

    # LLVM
    if [ "$TRAVIS_OS_NAME" = osx ]; then
        wget https://github.com/llvm/llvm-project/releases/download/llvmorg-9.0.1/clang+llvm-9.0.1-x86_64-apple-darwin.tar.xz
        tar xf clang+llvm-9.0.1-x86_64-apple-darwin.tar.xz && mv clang+llvm-9.0.1-x86_64-apple-darwin $HOME/clang9
    else
        wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
        tar xf clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz && mv clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04 $HOME/clang9
    fi

    # Boolector
    if [ -z "$(ls -A $HOME/boolector)" ]; then
        git clone https://github.com/boolector/boolector && cd boolector && git reset --hard 3.2.0 && ./contrib/setup-lingeling.sh && ./contrib/setup-btor2tools.sh && ./configure.sh --prefix $HOME/boolector-3.2.0 && cd build && make -s -j4 && make install
        cd $ROOT_DIR
    else
        echo "Boolector cache hit"
    fi

    # Z3
    if [ "$TRAVIS_OS_NAME" = linux ]; then
        wget https://github.com/Z3Prover/z3/releases/download/z3-4.8.4/z3-4.8.4.d6df51951f4c-x64-ubuntu-16.04.zip && unzip z3-4.8.4.d6df51951f4c-x64-ubuntu-16.04.zip && mv z3-4.8.4.d6df51951f4c-x64-ubuntu-16.04 $HOME/z3
        cd $ROOT_DIR
    fi

    # MathSAT
    if [ "$TRAVIS_OS_NAME" = linux ]; then
        wget http://mathsat.fbk.eu/download.php?file=mathsat-5.5.4-linux-x86_64.tar.gz -O mathsat.tar.gz && tar xf mathsat.tar.gz && mv mathsat-5.5.4-linux-x86_64 $HOME/mathsat
        cd $ROOT_DIR
    else
        wget http://mathsat.fbk.eu/download.php?file=mathsat-5.5.4-darwin-libcxx-x86_64.tar.gz -O mathsat.tar.gz && tar xf mathsat.tar.gz && mv mathsat-5.5.4-darwin-libcxx-x86_64 $HOME/mathsat
        # This is needed because CMake does not include /usr/local/include when trying to compile mathsat
        ln -s /usr/local/include/gmp.h $HOME/mathsat/include/gmp.h
        cd $ROOT_DIR
    fi

    # GMP build (linux)
    if [ -z "$(ls -A $HOME/gmp)" ] && [ "$TRAVIS_OS_NAME" = linux ]; then
        wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz && tar xf gmp-6.1.2.tar.xz && rm gmp-6.1.2.tar.xz && cd gmp-6.1.2 && ./configure --prefix $HOME/gmp --disable-shared ABI=64 CFLAGS=-fPIC CPPFLAGS=-DPIC && make -j4 && make install
        cd $ROOT_DIR
    else
        echo "GMP cache hit"
    fi

    # Yices 2
    if [ -z "$(ls -A $HOME/yices)" ]; then
        if [ "$TRAVIS_OS_NAME" = linux ]; then
            git clone https://github.com/SRI-CSL/yices2.git && cd yices2 && git checkout Yices-2.6.1 && autoreconf -fi && ./configure --prefix $HOME/yices --with-static-gmp=$HOME/gmp/lib/libgmp.a && make -j4 && make static-lib && make install && cp ./build/x86_64-pc-linux-gnu-release/static_lib/libyices.a $HOME/yices/lib
            cd $ROOT_DIR
        else
            git clone https://github.com/SRI-CSL/yices2.git && cd yices2 && git checkout Yices-2.6.1 && autoreconf -fi && ./configure --prefix $HOME/yices && make -j4 && make static-lib && make install && cp ./build/x86_64-apple-darwin*release/static_lib/libyices.a $HOME/yices/lib
            cd $ROOT_DIR
        fi
    else
        echo "Yices 2 cache hit"
    fi

    # CVC 4
    if [ -z "$(ls -A $HOME/cvc4-release)" ]; then
        git clone https://github.com/CVC4/CVC4.git && cd CVC4 && git reset --hard b826fc8ae95fc && ./contrib/get-antlr-3.4 && ./configure.sh --optimized --prefix=$HOME/cvc4-release --static --no-static-binary && cd build && make -j4 && make install        
        cd $ROOT_DIR
    else
        echo "CVC4 cache hit"
    fi
}

travis_script() {
    # Compile ESBMC

    export BASE_FLAGS="-DBUILD_TESTING=On -DENABLE_REGRESSION=On -DBUILD_STATIC=On -DClang_DIR=$HOME/clang9 -DLLVM_DIR=$HOME/clang9 -DCMAKE_INSTALL_PREFIX:PATH=$HOME/release"
    export SOLVERS="-DBoolector_DIR=$HOME/boolector-3.2.0 -DMathsat_DIR=$HOME/mathsat -DCVC4_DIR=$HOME/cvc4-release"
    export MAC_EXCLUSIVE="-DC2GOTO_INCLUDE_DIR=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/"

    if [ "$TRAVIS_OS_NAME" = osx ]; then
        mkdir build
        cd build
        cmake ..  $BASE_FLAGS $SOLVERS $MAC_EXCLUSIVE -DENABLE_Z3=On || echo "cmake warning"
        make -s -j4
    else
        mkdir build
        cd build
        cmake .. -GNinja $BASE_FLAGS $SOLVERS -DZ3_DIR=$HOME/z3
    fi

}

travis_after_success() {
    ccache -s
}

set -e
set -x

$1