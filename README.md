## Fuego on iOS

The upstream [Fuego](http://fuego.sourceforge.net/) project is a collection of C++ libraries for developing software for the game of Go.

The primary goal of the Fuego on iOS project is to provide a build of the Fuego source code for the iOS platform. The build is packaged into a framework bundle that can be easily integrated into any Xcode project. The build tries to remain as close as possible to the original upstream source code, with only such modifications as are necessary to overcome any difficulties in 1) building for the iOS platform, and in 2) building with a modern version of Boost.

A secondary goal of the Fuego on iOS project is to provide an iOS build of Fuego that is suitable for integration into the [Little Go](https://github.com/herzbube/littlego) project.

Fuego on iOS currently combines

* Boost 1.54.0
* Fuego trunk r1709

## Quickstart Guide

The project is set up with the `fuego-on-ios` branch as the default branch. This should allow you to get going immediately.

1. Clone the repository
1. Initialize the "boost-trunk" submodule. Find the necessary commands further down in the section "The boost-trunk submodule".
1. Build Boost, then build Fuego. Find the necessary commands further down in the section "How to build".

That's it, now all you need to do is integrate the Boost and Fuego frameworks into your project.

## Branches

The Fuego on iOS project has 3 permanent branches. Each of these branches is dedicated to exactly one task required to achieve the project goals.

##### master

Fuego on iOS uses `git svn` to track the [upstream Subversion repository](http://svn.code.sf.net/p/fuego/code/). The purpose of the `master` branch is to track the upstream `trunk`. No commits are allowed in `master` except those made by `git svn` to synchronize with upstream. The reason for this hard rule is that `git svn` will rebase local commits on top of Subversion commits, which will lead to problems due to published history being changed.

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

You should use `master` if you are a Git user who wants to work with the unmodified Fuego source code, but cannot be bothered to fiddle with Subversion or `git svn`. The `master` branch simply takes care of "wrapping" the upstream Subversion repository in a Git repository. Currently the disadvantage is that the `master` branch may not be up-to-date because it must be manually synchronized.

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

Current build settings:

* iOS SDK 6.1
* Deployment target 5.0
* C++ Language Dialect = GNU++98 (`-std=gnu++98`)
* C++ Standard Library = libstdc++ (`-stdlib=libstdc++`)
* Boost libraries: thread, filesystem, program_options, system, test, date_time

## Repository maintenance

These are some notes on how to maintain the Fuego on iOS repository and its branches.

##### Synchronizing master with upstream Fuego

Checkout `master`, then run this command:

    git svn rebase
    
If this gives you an error message, you should probably read the section "Reconnecting a cloned Git repository with upstream Subversion repository" further down.

##### Integrating changes from master into fuego-on-ios

All commits in `master` up to the desired commit must be merged into the `fuego-on-ios` branch. There is no cherry-picking to keep things simple.

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

##### Reconnecting a cloned Git repository with upstream Subversion repository

When you clone the Fuego on iOS repository from GitHub, you will not be able to use `git svn` to synchronize the local repository with upstream Fuego. The reason is that certain configuration information required by `git svn` is kept only locally and is not versioned in Git, i.e. this information cannot be shared via GitHub. Therefore, anyone who wants to use `git svn` locally must first reconnect their local clone of `fuego-on-ios` to the upstream Subversion repository.

The indicator that you are affected by this is when you issue a `git svn` command and you get the error message "Unable to determine upstream SVN information from working tree history".

The first step to fix the problem is to add some configuration information to the `.git/config` file of your local repository:

    cd /path/to/fuego-on-ios
    git config svn-remote.svn.url http://svn.code.sf.net/p/fuego/code
    git config svn-remote.svn.fetch trunk:refs/remotes/trunk
    git config svn-remote.svn.branches branches/*:refs/remotes/*
    git config svn-remote.svn.tags tags/*:refs/remotes/tags/*

Now checkout `master` if you have not yet done so. Look at the output of `git log`, and note down the SHA hash of the most recent commit that represents a Subversion commit (the commit message of such a commit has a line in it that starts with "git-svn-id"). 

Run the following command with the SHA hash you previously noted down:

    echo <hash> >.git/refs/remotes/trunk

The final step is to restore the contents of the `.git/svn` folder. This is done automatically by this command:

    git svn info

(other commands such as `git svn fetch` may also work). From now on, `git svn rebase` should work as expected.

##### How the Fuego on iOS repository was initialized

This purely historical information documents how the `fuego-on-ios` repository was initialized.

    mkdir fuego-on-ios
    cd fuego-on-ios
    git svn init --stdlayout http://svn.code.sf.net/p/fuego/code/ .
    git svn fetch

Besides the master branch that tracks the upstream trunk, these commands also create a number of remote tracking branches, one for each tag and branch in the upstream repo. They can be listed with `git branch -a`. These remotes are visible only locally and are not available to people who clone the public repository from GitHub.

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

## Attributions

The original "Boost on iPhone" build script is (c) Copyright 2009 Pete Goodliffe. Check out the [original article](http://goodliffe.blogspot.ch/2010/09/building-boost-framework-for-ios-iphone.html) that presents the script. The comments below the article have useful pointers to derivates of the script.

Daniel Sefton and Joseph Galbraith made substantial improvements to the "Boost on iPhone" build script. The improvements are presented in [this article](http://www.danielsefton.com/2012/03/building-boost-1-49-with-clang-ios-5-1-and-xcode-4-3/). Again, the comments below the article have useful pointers to derivates of the script.

The script that builds Fuego as a framework is based on [this work](https://github.com/justinweiss/fuego-framework) by Justin Weiss.

Thank you, everybody!