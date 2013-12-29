## Fuego on iOS

The upstream [Fuego](http://fuego.sourceforge.net/) project is a collection of C++ libraries for developing software for the game of Go.

The primary goal of the Fuego on iOS project is to provide a build of the Fuego source code for the iOS platform. The build is packaged into a framework bundle that can be easily integrated into any Xcode project. The build tries to remain as close as possible to the original upstream source code, with only such modifications as are necessary to overcome any difficulties in 1) building for the iOS platform, and in 2) building with a modern version of Boost.

A secondary goal of the Fuego on iOS project is to provide an iOS build of Fuego that is suitable for integration into the [Little Go](https://github.com/herzbube/littlego) project.

Fuego on iOS currently combines

* Boost 1.55.0
* Fuego trunk r1728

## Quickstart Guide

The project is set up with the `fuego-on-ios` branch as the default branch. This should allow you to get going immediately:

1. Clone the repository
1. Initialize the "boost-trunk" submodule. Find the necessary commands further down in the section "[The boost-trunk submodule](#the-boost-trunk-submodule)".
1. Build Boost, then build Fuego. Find the necessary commands further down in the section "[How to build](#how-to-build)".

That's it, now all you need to do is integrate the Boost and Fuego frameworks into your project.

## Branches

##### Overview

The Fuego on iOS project has 3 permanent task branches:

* master
* fuego-on-ios
* fuego-for-littlego

The next sections explain the purpose of each of these task branches. There are also a number of branches that correspond to upstream Subversion branches. These branches are of no particular interest to the Fuego on iOS project and exist simply to mirror the upstream repository structure.

##### master

Fuego on iOS uses `svn2git` to mirror the [upstream Subversion repository](http://svn.code.sf.net/p/fuego/code/). The purpose of the `master` branch is to track the upstream `trunk`. No commits are allowed in `master` except those made by `svn2git` to synchronize with upstream. The reason for this hard rule is that `svn2git` will rebase local commits on top of Subversion commits, which would lead to problems due to published history being changed.

Currently, upstream changes are manually synchronized from time to time. A future goal is to automate this process (see issue #1 on GitHub).

##### fuego-on-ios

The `fuego-on-ios` branch is dedicated to maintaining the following stuff:

1. `boost/boost-trunk`: A Git submodule that tracks a specific release of Boost in [this upstream repository](https://github.com/ned14/boost-trunk).
1. `boost/boost.sh`: A script that builds Boost for the iOS platform and packages it into a framework bundle. The script is a derivate of Pete Goodliffe's "Boost on iPhone" script. The script was forked from [this popular repo](https://gitorious.org/boostoniphone/galbraithjosephs-boostoniphone) so that outstanding bugs could be fixed for the Fuego on iOS project.
1. `fuego-on-ios.xcodeproj`: The Xcode project that defines the build of Fuego for the iOS platform.
1. `build.sh`: A script that uses the Xcode project to build Fuego (via `xcodebuild`) and packages the result into a framework bundle.

Also in this branch are those changes to the original Fuego source code that are necessary to make Fuego compile in the Xcode environment.

##### fuego-for-littlego

The branch `fuego-for-littlego` contains those changes to the Fuego source code that are required by the [Little Go](https://github.com/herzbube/littlego) project.

These changes are separated into their own branch because the intent for the `fuego-on-ios` branch is to keep that build as close to the original source as possible so that other people can take it from there and add their own stuff without the Little Go modifications (which might not be palatable to everyone).

## Which branch should I use?

You should use `master` if you are a Git user who wants to work with the unmodified Fuego source code, but cannot be bothered to fiddle with Subversion or `svn2git`. The `master` branch simply takes care of "wrapping" the upstream Subversion repository in a Git repository. Currently the disadvantage is that the `master` branch may not be up-to-date because it is manually synchronized.

You should use `fuego-on-ios` if you want a ready-to-build Fuego + Boost environment. The build products are two framework bundles (Fuego + Boost) that you can simply add to your iOS application project.

You probably will not be interested in `fuego-for-littlego` at all, unless you want to study how the [Little Go](https://github.com/herzbube/littlego) app integrates Fuego.

## The boost-trunk submodule

The `fuego-on-ios` branch (and also the `fuego-for-littlego` branch) contain a Git submodule in this project folder:

    boost/boost-trunk

The submodule name is derived from the name of the upstream Git repository (https://github.com/ned14/boost-trunk). Despite the name, the submodule does not point to Boost's upstream trunk, it actually points to a specific Boost release. After you clone the Fuego on iOS repository, you must perform the following commands to also clone the submodule:

    cd /path/to/fuego-on-ios
    git submodule init
    git submodule update

**IMPORTANT:** Although the submodule tracks only a single tag of the upstream Git repository, cloning requires that the full upstream repo is replicated locally. Because the Boost project has a long history, the resulting download is quite large. At the time of writing the download size was 362 MB, and on my machine the cloning operation took roughly 10 minutes (including download time at about 1.2 MB/s).

## How to build

These are the commands to first build Boost, then build Fuego:

    cd boost
    ./boost.sh
    cd ..
    ./build.sh

These are the build results that can be integrated into other Xcode projects:

    boost/ios/framework/boost.framework
    ios/framework/fuego-on-ios.framework

Most important build settings:

* iOS SDK = The latest SDK known to your Xcode
* Deployment target 5.0
* C++ Language Dialect = GNU++98 (`-std=gnu++98`)
* C++ Standard Library = libstdc++ (`-stdlib=libstdc++`)
* Boost libraries: thread, filesystem, program_options, system, test, date_time

## Repository maintenance

These are some notes on how to maintain the Fuego on iOS repository and its branches.

##### Synchronizing master with upstream Fuego

Checkout `master`, then run this command:

    svn2git --metadata --rebase

If you don't have `svn2git` in your environment, read the section "[Getting svn2git](#getting-svn2git)". If you *do* have svn2git, but running the command gives you an error message, you should probably read the section "[Reconnecting a cloned Git repository with upstream Subversion repository](#reconnecting-a-cloned-git-repository-with-upstream-subversion-repository)" further down.

In an ideal world, the `--rebase` command line option would update the local Git repository so that it is an exact mirror of the upstream Subversion repository. Unfortunately, in the `svn2git` version that is current at the time of writing (v2.2.2) the mirroring capability of `--rebase` is severely limited. In fact, the command line option does not do much else besides fetching upstream commits into the local Git repository. The following tasks need to be done manually:

1. Add local tags for tags that were created upstream since the last sync
    1. List all remote branches: `git branch -r`
    1. List all tags: `git tag`
    1. Pick out remote SVN branches whose name starts with `svn/tags` for which there is no corresponding tag
    1. For every branch thus found, get a log message: `git show svn/tags/[TAG-NAME]`
    1. Create the corresponding tag: `git tag -a -m "[LOG MESSAGE]" [TAG-NAME] svn/tags/[TAG-NAME]`
    1. Remove the remote branch: `git branch -rd svn/tags/[TAG-NAME]`
1. Add local branches for branches that were created upstream since the last sync
    1. List all branches: `git branch -a`
    1. Pick out remote SVN branches for which there is no corresponding local branch
    1. For every branch thus found, create a local branch: `git branch [BRANCH-NAME] remotes/svn/[BRANCH-NAME]`
1. Advance the HEAD of all existing branches that received commits upstream since the last sync. Usually, this affects the `master` branch.
   1. This affects all local branches where HEAD does not point to the same commit as HEAD of the corresponding remote branch
   1. Check out each affected branch, then advance its HEAD: `git reset --hard [COMMIT-REF]`

Once the local Git repository is in shape, the changes can be pushed to GitHub.


##### Integrating changes from master into fuego-on-ios

All commits in `master` up to the desired commit must be merged into the `fuego-on-ios` branch. To keep things simple no cherry-picking is allowed.

After the merge, the Xcode project may also need to be updated (add new files, remove old files, update build settings).

After `fuego-on-ios` has been updated, changes must be further merged into the `fuego-for-littlego` branch.

##### Upgrading to a new Boost release

To upgrade to a new Boost release, the `boost-trunk` submodule must be changed so that it points to the commit of the Git tag that represents the desired release. This must be done on the `fuego-on-ios` branch. For instance, to upgrade to version 1.54.0:

    cd /path/to/fugo-on-ios
    git checkout fuego-on-ios
    cd boost/boost-trunk
    git checkout release/Boost_1_54_0
    cd ../..
    git add .
    git commit -m ""upgrade submodule boost-trunk to tag release/Boost_1_54_0"

There is a possibility that the Fuego source code does not build with the new Boost release. If such a problem occurs, check with Fuego upstream if they already know a solution.

After `fuego-on-ios` has been updated, changes must be further merged into the `fuego-for-littlego` branch.

##### Making changes in fuego-for-littlego

All changes in the `fuego-for-littlego` branch remain local to that branch.

##### Updating this README

Updates to this README file are made on the `fuego-on-ios` branch and then merged into `fuego-for-littlego`. The README file does not exist in `master`.

## Troubleshooting

##### Getting svn2git

`svn2git` is a Ruby wrapper around `git svn`. Source code and installation instructions are available from [this GitHub repository](https://github.com/nirvdrum/svn2git). The Fuego on iOS project uses `svn2git` because it adds two bits of magic to `git svn`:

* `svn2git` creates native Git branches and tags from Subversion branches and tags, whereas `git svn` just imports everything into the `master` branch and creates remote tracking branches for Subversion branches and tags.
* `svn2git` points HEAD of `master` to the commit that represents the latest Subversion revision in the Subversion repo's trunk, whereas `git svn` points HEAD of `master` to the most recent Subversion revision, in whatever branch that may be (not necessarily trunk).

As of this writing, the latest released version of `svn2git` is 2.2.2. This version has a problem interacting with Git releases 1.8.3.2 and later. If you have this configuration, you can either downgrade Git on your system to 1.8.3.1, or you can get an unreleased version of svn2git where [this fix](https://github.com/nirvdrum/svn2git/issues/132) has been integrated.
  
##### Reconnecting a cloned Git repository with upstream Subversion repository

When you clone the Fuego on iOS repository from GitHub, you will not be able to use `svn2git` to synchronize the local repository with upstream Fuego. The reason is that certain configuration information required by `svn2git` (actually `git svn`) is kept only locally and is not versioned in Git, i.e. this information cannot be shared via GitHub. Therefore, anyone who wants to use `svn2git` must first reconnect their local clone of `fuego-on-ios` to the upstream Subversion repository.

The indicator that you are affected by this is when you issue a `svn2git` command and you get the error message "command failed: 2>&1 git svn fetch".

To fix the problem, add some configuration information to the `.git/config` file of your local repository:

    cd /path/to/fuego-on-ios
    git config svn-remote.svn.url http://svn.code.sf.net/p/fuego/code
    git config svn-remote.svn.fetch trunk:refs/remotes/trunk
    git config svn-remote.svn.branches branches/*:refs/remotes/*
    git config svn-remote.svn.tags tags/*:refs/remotes/tags/*

Now checkout `master` if you have not yet done so, then run

    svn2git --rebase

This takes quite a long time because it fetches all revisions from upstream Subversion. Behind the scenes the contents of the `.git/svn` folder are restored. From now on, `svn2git --rebase` should work as expected.

## Recipes

##### Build Fuego for Mac OS X

Assuming you have Boost installed somewhere locally (e.g. through Fink, MacPorts or Brew), you can run these commands to build and run Fuego for Mac OS X:

    cd /path/to/fuego-on-ios
    autoreconf -i
    ./configure --prefix $(pwd)/macosx
    make
    make install
    ./macosx/bin/fuego

Such a build may be useful to test patches on the command line.

##### How the Fuego on iOS repository was initialized

This purely historical information documents how the `fuego-on-ios` repository was initialized.

    mkdir fuego-on-ios
    cd fuego-on-ios
    svn2git --metadata http://svn.code.sf.net/p/fuego/code/

##### How the Xcode project was created

Project creation:

* Create new Xcode project with Xcode 4.6
* Template = Cocoa Touch Static Library
* Name = fuego-on-ios

File changes:

* Remove all files that were automatically added to the project by the Xcode template
* Add the following subfolders with their content to the project: fuegomain, go, gouct, gtpengine, simpleplayers, smartgame. Select the option "Create groups for any added folders". With this option it is possible to later remove individual files.
* Remove all "test" subfolders (e.g. go/test) and all "Makefile.am" files (e.g. go/Makefile.am) from the project
* Remove FuegoMain.cpp from the target "fuego-on-ios" (contains the main() function)
* Remove SgProcess.cpp and SgProcess.h from the target "fuego-on-ios" (fixes a compiler error in SgProcess.h, where a GCC specific header is included)
* Add all .cpp and .h files to the target "fuego-on-ios"

Project settings changes:

* Target "fuego-on-ios" > Build Phases > Add Build Phase "Copy Headers", then drag all .h files from the project tree into the "Public" section of the build phase
* Target "fuego-on-ios" > Build Phases > In build phase "Copy Headers" drag all .h files from the "Project" section to the "Public" section
* Target "fuego-on-ios" > Build Phases > Link Binary With Libraries > Select the Boost framework previously built (path = `boost/ios/framework/boost.framework`)
* Target "fuego-on-ios" > Build Settings > Precompile Prefix Header = No
* Target "fuego-on-ios" > Build Settings > Prefix Header = Clear
* Project > iOS Deployment Target = iOS 5.0
* Project > Public Headers Folder Path = Headers
* Project > C++ Language Dialect = GNU++98 (-std=gnu++98)
* Project > C++ Standard Library = libstdc++ (-stdlib=libstdc++)
* Project > Inline Methods Hidden = Yes (-fvisibility-inlines-hidden)
* Project > Symbols hidden by Default = Yes (-fvisibility=hidden)
* Project > Preprocessor Macros = NDEBUG (fixes a runtime error in case Fuego is run with the command line option `--quiet`)

## Attributions

The original "Boost on iPhone" build script is (c) Copyright 2009 Pete Goodliffe. Check out the [original article](http://goodliffe.blogspot.ch/2010/09/building-boost-framework-for-ios-iphone.html) that presents the script. The comments below the article have useful pointers to derivates of the script.

Daniel Sefton and Joseph Galbraith made substantial improvements to the "Boost on iPhone" build script. The improvements are presented in [this article](http://www.danielsefton.com/2012/03/building-boost-1-49-with-clang-ios-5-1-and-xcode-4-3/). Again, the comments below the article have useful pointers to derivates of the script.

The script that builds Fuego as a framework is based on [this work](https://github.com/justinweiss/fuego-framework) by Justin Weiss.

Thank you, everybody!