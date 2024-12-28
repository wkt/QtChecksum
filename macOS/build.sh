#!/bin/bash

realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

SELFDIR=$(dirname $(realpath "$0"))
BUILD_DIR=${SELFDIR}/../build-macOS
QT_DIR=$(ls -d ~/Applications/Qt/[0-9.]*|head -n1)
QMAKE="${QT_DIR}"/clang_64/bin/qmake
MACDEPLOYQT="${QT_DIR}"/clang_64/bin/macdeployqt
MAKE=make

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

rm -rf ../ui_*.h
${QMAKE} ${SELFDIR}/../QtChecksum.pro -spec macx-clang CONFIG+=release CONFIG+=X86_64 CONFIG+=qml_release
${MAKE} -f ${BUILD_DIR}/Makefile qmake_all

${MAKE} -f ${BUILD_DIR}/Makefile all
${MACDEPLOYQT} ${BUILD_DIR}/QtChecksum.app -dmg
