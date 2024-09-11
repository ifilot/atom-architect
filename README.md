# Atom Architect

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ifilot/atom-architect?label=version)
[![build](https://github.com/ifilot/atom-architect/actions/workflows/build.yml/badge.svg)](https://github.com/ifilot/atom-architect/actions/workflows/build.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

![Screenshot of Atom Architect](img/atom-architect-screenshot.JPG)

## Purpose
Atom Architect is a VASP visualization and structure building tool. Its unique
feature is that atom placement is conducted in a relative fashion, i.e. with
respect to the existing atomic structure. This is especially useful for
catalysis purposes where atoms are typically placed at e.g. bridge, threefold or
fourfold sites.

## Downloads

Latest installer for Window: [here](https://github.com/ifilot/atom-architect/releases/latest/download/atom-architect-installer-win64.exe)

## Compilation

For Windows, it is recommended to use the installer as shown above. For Linux,
it is recommended to compile Atom Architect yourself. Please note that Atom
Architect is only tested for Debian 12 and Ubuntu 22.04. For other
distributions, you are on your own. 

To compile Atom Architect, please follow the compilation instructions as shown
below.

Start by installing all the required dependencies

```bash
sudo apt update && sudo apt install -y \
qt6-tools-dev \
qt6-base-dev \
libqt6charts6-dev \
libqt6widgets6 \
libqt6gui6 \
libqt6opengl6-dev \
libgl1-mesa-dev \
build-essential \
cmake
```

After having cloned this repository and starting at its root folder, execute

```bash
mkdir build
cd build
cmake ../
make -j
```

This will generate the compilation scripts and compile Atom Architect. You can
use Atom Architect by running `./atom_architect` in your `build` folder. If
you wish to install Atom Architect on your system, you can run in your `build`
folder the following command.

```bash
sudo cp -v ./atom_architect /usr/local/bin/atom_architect
```

## Dependencies

Atom Architect depends on [GLM](https://github.com/g-truc/glm) and
[Eigen3](https://eigen.tuxfamily.org/index.php?title=Main_Page). Both are
header-only C++ libraries and are for convenience purposes added to this
repository. This implies that compilation of Atom Architect will not use your
system-installed version of GLM and Eigen3, but use the one provided in this
repository.

## Common problems

> I have troubles running `Atom Architect` remotely via MobaXterm.

Try to enable direct rendering by setting the following environmental variable:

```bash
export LIBGL_ALWAYS_INDIRECT=0
```