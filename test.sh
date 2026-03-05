#!/bin/env bash

RC='\033[0;0m'
C1='\033[0;31m'
C2='\033[0;32m'
C3='\033[0;33m'
C4='\033[0;34m'
C5='\033[0;35m'
C6='\033[0;36m'

nTestsGlobal=0
nPassedGlobal=0
verbose=0

testSets="paxos"

run_test() {
  if [ -f $1.out ]
  then
    ./$1.out &> $1.log

    if [ $? = 0 ]
    then
      echo -e "    ${C2}Passed.${RC}"
      return 0
    else
      echo -e "    ${C1}Failed.${RC}"
      return 1
    fi
  else 
    echo -e "    ${C1}Missing.${RC}"
    return 2
  fi
}

run_test_set() {
  tests=$(ls $1*.cc | rev | cut -c 4- | rev)
  nTests=$(ls $1*.cc | wc -l)
  ((nTestsGlobal+=nTests))
  nPassed=0
  
  echo -e "\n${C5}Running $1 tests${RC}:"

  for test in $tests
  do 
    echo -e "${C3}${test}:${RC}" | sed 's/_/ /g'
    if [ "$verbose" = "1" ]
    then
      cat ./${test}.cc | grep " \* "
    fi

    run_test $test

    if [ $? = 0 ]
    then
      ((nPassed++))
      ((nPassedGlobal++))
    fi
  done

  echo -e "${C4}${nPassed}/${nTests} testcases passed.${RC}"
  return $nTests
}

cd ./test

if [ $# = 1 ] && [ "$1" = "-v" ] 
then
  verbose=1
elif [ $# != 0 ] 
then 
  testSets=$@
fi

for arg in $testSets
do
  if [ $arg = "-v" ]
  then
    verbose=1
  else 
    run_test_set $arg
  fi
done

if [ $nTests != $nTestsGlobal ] 
then
  echo -e "\n${C5}Total${RC}:"
  echo -e "${C6}${nPassedGlobal}/${nTestsGlobal} testcases passed.${RC}"
fi

exit $(( $nTestsGlobal - $nPassedGlobal ))

