Introduction:
-------------

This document provides important information about the Android
emulator codebase, which will be useful for anyone planning to
contribute patches to it.


I. Getting the sources:
-----------------------

I.1) Get the 'repo' tool:
- - - - - - - - - - - - -

See https://source.android.com/source/downloading.html for download and
installation instructions.

I.2) Determine the current Android Studio development branch:
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

See the content of http://tools.android.com/build

As of this writing, the major emulator development happens on emu-master-dev

I.3) Checkout the sources with 'repo':
- - - - - - - - - - - - - - - - - - - -

    mkdir emu-master-dev
    cd emu-master-dev
    repo init -u https://android.googlesource.com/platform/manifest \
            -b emu-master-dev
    repo sync

Replace 'emu-master-dev' with another branch name if necessary.


II. Building the binaries:
--------------------------

II.1) Prebuilt binaries:
- - - - - - - - - - - -

Building the emulator now depends on many prebuilt libraries that are stored
under $TOP/prebuilts/android-emulator-build/. Normally, you don't need to
care about them, but read the section below named "Rebuilding prebuilts"
for more information, or if you need to add a new dependency.

II.2) Build the emulator binaries:
- - - - - - - - - - - - - - - - - -

After building all the dependencies, do:

  cd $TOP/external/qemu
  ./android/rebuild.sh

Note that android/rebuild.sh will build all sources, but also run a small
series of unit-tests to check that everything works as expected. In case
of problem, use the --verbose flag to see what's failing.

Final binaries will be located under the 'objs' directory. You can select
a different location with --out-dir=<path>.

All scripts available to build emulator binaries take a --help option that
describe them in detail, and provide an explanation of all supported options.

On Linux, it is possible to build Windows binaries by using the --mingw option
as in:

  ./android/rebuild.sh --mingw

If you have Wine installed, android/rebuild.sh will detect it and run the
unit-tests with it.

IMPORTANT: Unfortunately, it's not possible to run the Windows emulator
           binaries under Wine anymore. This seems related to OpenGL API
           emulation issues in Wine.

android/rebuild.sh also supports other interesting options (e.g. --debug to
generate debug versions of the binaries). For all details, use:

  ./android/rebuild.sh --help

And read the output.


II.3) Packaging binary releases:
- - - - - - - - - - - - - - - - -

You can create redistributable tarballs by using:

  cd $TOP/external/qemu
  android/scripts/package-release.sh

This will create /tmp/android-emulator-<date>-<system>.tar.bz2 containing
all binaries, ready to be uncompressed into an Android SDK installation
directory.

Alternatively, you can update the prebuilt binaries that are stored in
an AOSP checkout under:

   $AOSP/prebuilts/android-emulator/

These are the binaries that are launched when you type 'emulator' just
after a platform build, in the root AOSP directory.

To update them, do:

  android/scripts/package-release.sh  --copy-prebuilts=/path/to/aosp

Note that this also creates a README file in the target directory that
describes the git log of each source git tree since the last update.


II.4) Remote darwin builds:
- - - - - - - - - - - - - -

The scripts describe above work on Linux and Darwin systems only.

On Linux, binaries for both Linux and Windows are generated by default.

You can however build Darwin binaries from Linux, provided that you have
a Darwin machine available through SSH. To do that, do either one of the
following:

  - Set ANDROID_EMULATOR_DARWIN_SSH=<hostname> in your environment.
  - Use --darwin-ssh=<hostname> when calling one of the build scripts.

It is recommended to always use this when using package-release.sh, this
makes it an ideal way to check that a source code change actually compiles
for all 3 host platforms.


II.5) Speeding up builds with 'ccache':
- - - - - - - - - - - - - - - - - - - -

It is highly recommended to install the 'ccache' build tool on your development
machine(s). The Android emulator build scripts will probe for it and use it
if available, which can speed up incremental builds considerably.

Build scripts also support the --ccache=<program> and --no-ccache options to
better control ccache usage.



III. Testing:
-------------

III.1) Unit-testing:
- - - - - - - - - - -

./android/rebuild.sh already runs a small set of unit test programs and
verification checks. If they fail, it will print an error message about it.

Most unit-tests are in C++ and use the GoogleTest library. Whenever you add
a new Android-specific feature to the code base, that doesn't directly
modifies the QEMU-based code, add corresponding unit tests to avoid
regressions in the future.


III.2) Booting existing AVDs:
- - - - - - - - - - - - - - -

Otherwise, a simple way to test a emulator code change is trying to boot an
existing AVD, by doing the following after calling android/rebuild.sh:

   export ANDROID_SDK_ROOT=/path/to/installed/android/sdk
   objs/emulator <your-options>

III.3) Booting an Android platform build:
- - - - - - - - - - - - - - - - - - - - -

It is also possible to try booting an Android system image built from a fresh
AOSP platform checkout. One has to select an emulator-compatible build product,
e.g.:

   cd $AOSP/
   . build/envsetup.sh
   lunch aosp_arm-userdebug
   make -j8

Recommended build products are 'aosp_<cpu>-userdebug' or 'aosp_<cpu>-eng',
where <cpu> can be one of 'arm', 'x86', 'mips', 'x86_64', 'arm64' or 'mips64'.

To boot the generated system image:

   cd $TOP/external/qemu
   ./android/rebuild.sh
   export ANDROID_BUILD_TOP=/path/to/aosp
   objs/emulator <your-options>


IV. Overall source layout:
--------------------------

IV.1) Common Android-emulation related sources:
- - - - - - - - - - - - - - - - - - - - - - - -

The following directories contain generic libraries used for Android
emulation. By 'generic', it is meant that the corresponding sources
do not depend on a specific emulation engine (e.g. QEMU1 or QEMU2 as
described below).

Under $TOP/external/qemu/ they are located at:

  android/android-emu/
    Android-specific code, used to implement a UI and functional layer on
    top of the QEMU code base. Ideally, anything that shouldn't impact the
    QEMU internals should be here.

  android/android-emugl/
    Android-specific GPU emulation library, also called "EmuGL".
    Read the DESIGN.TXT file in this directory for more information
    on its design and how it is used at runtime. Also read
    android/docs/GPU-EMULATION.TXT.

  android/third_party/
    Contains various third-party libraries build scripts and/or sources that
    the Android emulation libraries depend on (e.g. zlib, libpng, libSDL).


IV.1) "Classic" emulator source layout:
- - - - - - - - - - - - - - - - - - - -

The 'classic' emulator programs are used to run legacy Android SDK system
images (from Cupcake to Lollipop) and are based on a very ancient version of
QEMU1 (1.10) that has been drastically patched and altered.

Its sources lie under $TOP/external/qemu/android/qemu1/, with the following
important sub-directories:

  android-qemu1-glue/
    Glue code between QEMU1 and the Android emulation libraries. Ideally,
    it is the only part of the code that is supposed to know about both
    android/android-emu/ and android/qemu1/ at the same time.
    (In practice some of the android/qemu1/ includes android/android-emu/
    headers directly, but since this is a mostly-deprecated version of
    QEMU, we don't really bother about cleaning this up).

  include/hw/android/goldfish/
    Headers for the Android-specific goldfish virtual devices.
    See docs/GOLDFISH-VIRTUAL-PLATFORM.TXT for more details.

  hw/android/goldfish/
    Implementation files for the Android-specific goldfish virtual devices.

  hw/android/
    Implementation files for the Android-specific virtual ARM and MIPS
    boards. For x86, the content of hw/i386/pc.c was modified instead.

  slirp-android/
    Modified version of the slirp/ directory, which adds various Android-specific
    features and modifications.

Generally speaking, some QEMU source files have been rewritten in so signficant
ways that they gained an -android prefix (e.g. vl-android.c versus vl.c). The
original file is typically kept as a reference to make it easier to see
modifications and eventually integrate upstream changes.

Note that these sources are considered 'legacy', i.e. there is no effort to
port Android-specific changes here to upstream QEMU.


IV.2) New 'qemu2' emulator source layout:
- - - - - - - - - - - - - - - - - - - - - - - - -

The 'new' emulator programs are used to run more version Android system images
(e.g. Lollipop on 64-bit ARM CPUs), and are based on a much more recent version
of upstream QEMU (2.7.0 at this time), which has been slightly modified with
Android-specific changes.

The sources are under $TOP/external/qemu/, and match the upstream QEMU source
tree layout, to ease future rebases and cherry-picks of upstream fixes. This
is why all other sources are under $TOP/external/qemu/android/ too.

Note that these sources will be periodically rebased on top of upstream QEMU
changes / releases. It is our goal to ultimately send patches upstream to
enable Android support in QEMU, but this is lower priority task compared with
getting things to work.

One of the major differences between the classic and new code bases is that
they do not support the same virtual hardware.

- The classic emulator supports the 'goldfish' virtual platform.
  See docs/GOLDFISH-VIRTUAL-HARDWARE.TXT for a full description.

- The new emulator supports the 'ranchu' virtual platform, which is an
  evolution of 'goldfish' that replaces NAND and MMC devices with
  virtio-block instead, as well as a few other differences.

NOTE: 'ranchu' is still a work in progress and isn't completely documented
      yet, for full details, one should refer to the sources under
      external/qemu as well as the android-goldfish-3.10 branch
      found on https://android.googlesource.com/kernel/goldfish.git


V) Overview of all binaries:
----------------------------

The Android emulator is really made of several binaries that work together
to implement all required emulation features. This is a detailed description
of them:

  - The 'emulator' program launcher, which is the main entry point to
    launch a virtual Android device (AVD). Typically located under:

        $EXEC_DIR/emulator

    The role of this tiny program is to read the AVD's configuration, and
    device which specific emulation engine program to run (see below). It must
    also setup the environment to ensure that GPU emulation libraries are
    properly loaded when needed (which typically involves modifying
    LD_LIBRARY_PATH on Posix systems, and PATH on Windows).

  - CPU-specific "classic" (a.k.a QEMU1) emulation engines, which are
    independent program that can emulate a single virtual CPU. 32-bit
    and 64-bit versions are typically available, and are called by
    'emulator' based on AVD configuration, for example:

        $EXEC_DIR/emulator-arm   ->
                32-bit host program to emulate 32-bit ARM CPUs.

        $EXEC_DIR/emulator-x86   ->
                32-bit host program to emulate 32-bit x86 and
                64-bit x86_64 CPUs.

        $EXEC_DIR/emulator64-arm ->
                64-bit host program to emulate 32-bit ARM CPUs.

    These are tagged as "classic" because they are generated from the
    sources under external/qemu/android/qemu1, and can be used to emulate old
    Android system images (e.g. anything before the L release).

  - CPU-specific "newer" QEMU2 emulation engines, based on a newer version
    of QEMU (2.2.0), located under:

        $EXEC_DIR/qemu/$HOST_SYSTEM/qemu-system-$ARCH

    Where $HOST_SYSTEM is a host system tag (e.g. linux-x86_64), and $ARCH
    is a virtual cpu name, following QEMU convention (e.g. 'aarch64' for
    64-bit ARM CPUs).

    They are built by android/rebuild.sh from the sources under
    external/qemu/ that are _not_ under external/qemu/android/.

    Note that these are slightly-modified QEMU programs, and thus accept
    emulator-specific command-line options, not the original QEMU ones.

  - GPU Emulation libraries, which implement guest EGL/GLES to desktop GL
    translation. This is the default GPU emulation mode also known as 'host'
    mode. The files are:

        $EXEC_DIR/lib64/
                OpenglRender.so
                EGL_translator.so
                GLES_CM_translator.so
                GLESv2_translator.so

    The libraries are loaded by the emulation engine programs when needed
    (i.e. if the AVD has 'hw.gpu.enabled' set to 'yes'). In particular, the
    'emulator' launcher takes care of adding or $EXEC_DIR/lib64/ to
    LD_LIBRARY_PATH (or PATH on Windows) before launching the real emulation
    engine.

    There is also the possibility of supporting additional GPU emulation
    modes, through optional 'backends'. For example, the 'mesa' backend
    is used to perform software-based OpenGL rendering, and can be activated
    by using '-gpu mesa' or setting 'hw.gpu.mode' to 'mesa' in the AVD
    configuration.

    The files of a given backend <name> must be under:

        $EXEC_DIR/lib64/gles_<name>/

    For more details, see docs/GPU-EMULATION.TXT

  - The 'query answering machine', a small application which only purpose is
    to answer queries from Android Studio:

        $EXEC_DIR/emulator-check

    Currently the only parameter is '-accel', which returns the status of
    the hypervisor support on the current machine (if emulator can use
    hardware acceleration).

VI. Rebuilding prebuilt binaries:
---------------------------------

The emulator prebuilt binaries are stored in git repositories under:

  $TOP/prebuilts/android-emulator-build/

In particular, this contains an 'archive' sub-directory:

  $TOP/prebuilts/android-emulator-build/archive/
       A directory containing a PACKAGES.TXT file describing all source
       tarballs needed to rebuild the prebuilts from scratch. The directory
       also contains copies of said tarballs to avoid re-downloading them
       from the Internet every time.

  $TOP/prebuilts/android-emulator-build/<name>/

    A directory containing the binaries for prebuilt dependency <name>.
    These binaries are generated from the sources from .../archive/ through
    scripts provided under $TOP/external/qemu/android/scripts/.

The most important files here are the following:

   external/android/scripts/download-sources.sh:
       A script used to parse PACKAGES.TXT and download or check all
       packages into the .../archive one.

       If one wants to add a new prebuilt library, start by updating
       PACKAGES.TXT, then run download-sources.sh to update the
       .../archive/ content.

       IMPORTANT: In other words: IF YOU UPDATE PACKAGES.TXT, ALWAYS RUN
                  download-sources.sh BEFORE RUNNING ONE OF THE BUILD
                  SCRIPTS BELOW.

    external/android/scripts/build-<name>.sh:
       A script used to rebuild a given set of prebuilt binaries.
       Note that by default, when invoked on Linux, this builds Linux
       and Windows binaries at the same time. It is also possible to
       build for all supported host platforms with the --darwin-ssh=<hostname>
       option, that will direct the script to perform a remote build on
       a Darwin machine through ssh.

       The corresponding binaries are then stored under one of:

           $TOP/prebuilts/android-emulator-build/<name>/<system>/
           $TOP/prebuilts/android-emulator-build/common/<name>/<system>/

       Where <name> is the prebuilt name, and <system> is a host system tag
       like windows-x86 or linux-x86_64.

    external/android/scripts/build-qt.sh:
       As a special exception, this script works like the other build-<name>.sh
       scripts, except that the Qt source tarball are not listed in
       PACKAGES.TXT, nor is a copy stored under .../archive/.

       The reason for this is simply that the archive file is too large
       (several hundreds MBs). As such, one should call this script with
       the --download option on the first time. This will grab the tarball
       from official Qt servers and put it under .../archive/ where it
       will *not* be put under git.

    external/android/scripts/build-qemu-android.sh:
       This is another special exception because this script does not build
       something that is put on git repositories. The purpose of this script
       is to rebuild the QEMU2 binaries from sources under
       $TOP/external/qemu, using the original QEMU2 configure/build
       system (unlike android/rebuild.sh, which uses the emulator build
       system to compile them). The main difference is that said binaries
       cannot use the AndroidEmu library (see the documentation for it in
       docs/ANDROID-EMULATION-LIBRARY.TXT).

       The reason to use this script is to ease the task of tracking
       upstream QEMU changes, and rebasing our own modifications on top
       of it (mainly because the Android-specific features added on top
       of it are way too large to be accepted by upstream).

       In particular, build-qemu-android.sh generates LINK-<system>.TXT
       files describing all the object files used during a link of the
       final QEMU2 binaries. These files are later processed by a helper
       script located under:

          external/qemu/android-qemu2-glue/ \
                  scripts/gen-qemu2-sources-mk.sh

       to generate the correspond Android .mk rules used with
       android/rebuild.sh, i.e.:

          external/qemu/android-qemu2-glue/ \
                  build/Makefile.qemu2-sources.mk

       The new QEMU2 binaries are copied into:

          $TOP/prebuilts/android-emulator-build/qemu-android/

       Note that this directory is *not* backed by a git repository.

VII. Other related sources:
---------------------------

A full SDK depends on other notable sources from different locations:

* https://android.googlesource.com/platform/sdk:

    This repository has an emulator/ directory that contains the sources for
    various emulator-related sources and binaries:

       sdk/emulator/opengl/
          No longer exists, but used to host the sources for the host-side
          GPU emulation libraries. These are now located under
          external/qemu/android-emugl instead. Read the DESIGN.TXT document
          in this directory to learn more about how GPU emulation works.

       sdk/emulator/mksdcard/
          Sources for the 'mksdcard' SDK host tool, used to generate an empty
          SDCard partition image that can be used by the emulator. This
          executable is never called or used by the Android emulator itself,
          but by the AVD Manager UI tool instead.

       sdk/emulator/snapshot/snapshot.img
          An empty QCOW2 partition image file, this is copied by the AVD
          Manager into a new AVD's content directory if the user enables
          snapshotting.

    The rest of sdk/ is not related to emulation and can be ignored here.


* https://android.googlesource.com/platform/device/generic/goldfish/

    Contains emulation support libraries that must be in the system image
    for emulation to work correctly. Essentially HAL (Hardware Abstraction
    Layer) modules for various virtual devices, and the GPU EGL/GLES
    system libraries that communicate with the host-side ones during
    emulation.

* https://android.googlesource.com/platform/prebuilts/qemu-kernel/

    Prebuilt kernel images configured to run under emulation. Read
    docs/ANDROID-KERNEL.TXT for more information.

* https://android.googlesource.com/platform/prebuilts/android-emulator/

    Prebuilt emulator and GPU emulation libraries for various host platforms.
    These are used only when building an emulator-compatible system image
    with the platform build (e.g. 'aosp_<cpu>-userdebug').

    The 'lunch' command adds this directory to the path so you can just
    type 'emulator' after a succesful build, and have the corresponding
    system image started under emulation.

    Updating these binaries requires using the script under
    android/scripts/package-release.sh with the --copy-prebuilts option
    (see above).