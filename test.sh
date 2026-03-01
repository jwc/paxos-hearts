#!/bin/env bash

cd ./test

tests=$(ls test*.cc | rev | cut -c 4- | rev)
nTests=$(ls test*.cc | wc -l)
#nTests=$(echo $tests | wc -l)

passed=0
#failed=0

C1='\033[0;34m'
C2='\033[0;0m'
C3='\033[0;32m'
C4='\033[0;31m'
C5='\033[0;36m'

#for i in $(seq 1 $nTests)
#for i in $(ls test*.cc | rev | cut -c 4- | rev | cut -c 5-)
#for i in $(ls Test*.cc | rev | cut -c 4- | rev)
#for i in $(echo $tests | rev | cut -c 4- | rev)
for i in $tests
do
  echo -e ${C1}$i:${C2} | sed 's/_/ /g'
  cat ./${i}.cc | grep " \* "

  #echo -e ${C1}Test $i:${C2} 
  #cat ./test${i}.cc | grep " \* "

  if [ -f ${i}.out ]
  then
    ./${i}.out &> ${i}.log

    if [ $? = 0 ]
    then
      echo -e "\t${C3}Passed.${C2}"
      ((passed++))
    else
      echo -e "\t${C4}Failed.${C2}"
      #((failed++))
    fi
  else 
    echo -e "\t${C4}Missing.${C2}"
  fi
  #./test${i}.out &> test${i}.log
  #./test${i}.out 

done

wait
echo -e ${C5}${passed}/${nTests} testcases passed.${C2}

