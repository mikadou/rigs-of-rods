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


cd doc/doxygen/html
git init
git config user.name "${GH_USERNAME}"
git config user.email "${GH_EMAILADDRESS}"

git remote add upstream "https://${GH_TOKEN}@github.com/${GH_USERNAME}/rigs-of-rods.git"
git fetch upstream
git reset upstream/gh-pages

git add -A
git commit -m "[AUTO] Update documentation."
git push upstream gh-pages
