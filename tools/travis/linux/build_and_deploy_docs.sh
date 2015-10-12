#!/usr/env bash

set -o nounset


# Build Doxygen documentation
# (requires CMake option BUILD_DOC_DOXYGEN=TRUE)
cd ./build
make doc-doxygen


# Clone branch where generated documentation resides (i.e. gh-pages branch).
# Additionally configure the user which will be used to push the updated
# documentation. The GH_USERNAME and GH_EMAILADDRESS environment variables are
# defined by the .travis.yaml configuration file.
git clone --single-branch -b gh-pages "git://github.com/${GH_USERNAME}/rigs-of-rods.git" gh-pages
cd gh-pages
git config user.name "${GH_USERNAME}"
git config user.email "${GH_EMAILADDRESS}"


# Delete existing documentation and replace it with the newly generated documentation.
git rm -r ./*
cp -R ../doc/doxygen/html/* .
git add -A
git commit -m "[AUTO] Updated documentation."


# Deploy the resulting changes by pushing them to the remote gh-pages branch.
# The new documentation is then available on the corresponding Github pages.
# Authentification is done via GH_TOKEN which is provided by the .travis.yaml configuration.
git push https://${GH_TOKEN}@github.com/${GH_USERNAME}/rigs-of-rods.git gh-pages
cd ../..

