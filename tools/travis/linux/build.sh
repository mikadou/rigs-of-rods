cmake -DROR_USE_MYGUI="TRUE" \
-DROR_USE_OPENAL="TRUE" \
-DROR_USE_SOCKETW="TRUE" \
-DROR_USE_PAGED="TRUE" \
-DROR_USE_CAELUM="TRUE" \
-DROR_USE_ANGELSCRIPT="TRUE" \
-DCMAKE_CXX_FLAGS="-O0" \
-DBUILD_DOC_DOXYGEN="TRUE"
.

make doc-doxygen 

echo "Cloning single branch gh-pages."
git clone --single-branch -b gh-pages "git://github.com/${GH_USERNAME}/rigs-of-rods.git" gh-pages-git

cd gh-pages-git
git config user.name "${GH_USERNAME}"
git config user.email "${GH_EMAILADDRESS}"

echo "Copying new generated documentation."
rm -rf ./*
cp -R ../doc/doxygen/html/* .

git --version
git add -A
git commit -m "[AUTO] Update documentation."

echo "Pushing updated documentation to remote gh-pages branch."
git push "https://${GH_TOKEN}@github.com/${GH_USERNAME}/rigs-of-rods.git" gh-pages
cd ..
