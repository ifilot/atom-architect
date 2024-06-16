#!/bin/sh

# clean any previous data
rm -rvf packages/com.vendor.product/data

# create folder
mkdir -v packages/com.vendor.product/data

# copy executable
cp -v ../build/release/atom-architect.exe packages/com.vendor.product/data

# run windeploy
/c/Qt/6.7.0/mingw_64/bin/windeployqt.exe packages/com.vendor.product/data/atom-architect.exe --release --force
cp -v /c/Qt/Tools/mingw1120_64/bin/libgomp-1.dll packages/com.vendor.product/data

# copy icon
cp -v ../assets/icon/atom_architect_256.ico packages/com.vendor.product/data/atom_architect_256.ico

# run script generators
python3 package.py

# run packaging tool
/c/Qt/Tools/QtInstallerFramework/4.8/bin/binarycreator.exe -c config/config.xml -p packages atom-architect-installer-win64.exe