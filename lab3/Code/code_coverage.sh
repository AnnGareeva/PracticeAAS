lcov=`readlink -f ./lcov/bin/lcov`
genhtml=`readlink -f ./lcov/bin/genhtml`
cd `find .. -name debug`
rm *.gcda
`find -name *.exe`
$lcov --rc lcov_branch_coverage=1 --directory . --capture --output-file coverage.info
$genhtml coverage.info --branch-coverage --output-directory out 
start out/index.html
