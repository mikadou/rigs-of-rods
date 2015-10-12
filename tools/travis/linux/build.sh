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


git clone --single-branch -b gh-pages "https://${GH_TOKEN}@github.com/${GH_USERNAME}/rigs-of-rods.git" gh-pages-git
git config user.name "${GH_USERNAME}"
git config user.email "${GH_EMAILADDRESS}"

cd gh-pages-git
rm -rf ./*
cp -R ../doc/doxygen/html/* .

git add -A
git commit -m "[AUTO] Update documentation."
git push upstream gh-pages
