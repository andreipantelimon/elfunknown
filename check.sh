#!/bin/bash

# Copyright 2018 Darius Neatu (neatudarius@gmail.com)

total=0
MAX_POINTS=100.0

tests=10
declare -a points=(10 10 10 10 10 10 10 10 10 10)

TIMEOUT=2s
EXE=snowfight

TESTS_DIR=tests
CLEAN=YES

VALGRIND=YES
VALGRIND_TIMEOUT_TIME=10s

function display_score()
{
    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    
    status=0
    if [ ! -z "$1" ]; then
        status=$1
    fi

    if (( status != 0 )); then   
      exit $status
    fi

    echo "=================>>>>>> Total: ${total}/${MAX_POINTS}  <<<<<<==================="

    if (( $(echo "$total==100.0" | bc -l ) )); then
        echo "======================>>>>>>  Ce boss!  <<<<<<======================="
    else
        CLEAN=NO
        echo "Compara rezultatele din tests/ref/*.ref cu tests/out/*.out sa vezi care e buba."
    fi


    # clean junk and pwp
    if [[ $CLEAN = "YES" ]]; then
        rm -f tests/out/*.out
    fi

    exit $status
}

function check_readme()
{
    README=README

    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    echo "========================>>>>>>    ${README}   <<<<<< ==================="

    score=0
    max_score=5

    if (( $(echo "$total == 0" |bc -l)  )); then
        echo "Punctaj ${README} neacordat. Punctajul pe teste este ${total}!"
    elif [ ! -f $README ]; then
        echo "${README} lipsa."
    elif [ -f $README ] && [ "`ls -l ${README} | awk '{print $5}'`" == "0" ]; then
        echo "${README} gol. (nice try)"
    else
        score=5
        total=$(bc <<< "$total + 5")
        echo "${README} detectat. Punctajul final se va acorda la corectare."
    fi

    echo "========================>>>>>> score: ${score}/${max_score}  <<<<<< ==================="
    printf "\n\n"
}

function check_cs_errors()
{
    python cs.py *.h *.c 2> tmp
    cnt_cs=`cat tmp | grep "Total errors found" | cut -d ':' -f2 | cut -d ' ' -f2`
}

function check_coding_style()
{
    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    echo "---------------------------------------------------------------------"
    echo "=====================>>>>>> Coding style <<<<<< ====================="

    cnt_cs=0
    check_cs_errors

    if (( cnt_cs > 0)); then
        total=$(bc <<< "$total -15")

        echo ""
        echo "${cnt_cs} erori de CS. (-15p)"
        echo ""
        
        cat tmp | tail -20

    else
        echo "Nu au fost detectate erori de coding style in mod automat. Respect!"
        echo "Pot apărea însă depunctări la corectarea manuală (vezi enunț)."
    fi
    rm -f tmp

    printf "\n\n"
}

function check_mem_leaks()
{
    MEM_ERR=-1
    if [ "$VALGRIND" != "YES" ]; then
        echo "Sar peste rularea cu valgrind (seteaza VALGRIND=YES pentru aceasta verificare)"
        MEM_ERR=0
        return
    fi
    echo "Rulare valgrind: ${IN} ..."

    cp ${IN} ${EXE}.in 
    cmd="time timeout ${VALGRIND_TIMEOUT_TIME} valgrind --leak-check=full --error-exitcode=1 -q ./$EXE"
    echo "===> $cmd"
    ($cmd) 2> time.err
    MEM_ERR=$?

    rm -f time.err
}

function test_homework() 
{
    test_index=-1
    for i in $(seq -f "%02g" 0 $(($tests-1)) ); do
      test_index=$((test_index + 1))
      test_points=${points[$test_index]}
      
      IN="tests/in/${i}-${EXE}.in"
      OUT="tests/out/${i}-${EXE}.out"
      REF="tests/ref/${i}-${EXE}.ref"
      cp $IN ${EXE}.in

      timeout $TIMEOUT ./$EXE
      
      if [[ ! $? -eq 0 ]]; then
        echo "Test ${i}: BUBA -    0 /  ${test_points}"
      else
          cp ${EXE}.out $OUT
          diff $REF $OUT &> /dev/null
          if [ 0 -eq $? ]; then
            echo "Test ${i}: OK        -  ${test_points} /  ${test_points} (punctaj initial)"
            check_mem_leaks $IN

            if [ "$MEM_ERR" != "0" ]; then
                echo "Test ${i}: MEM_ERR   -  0 /  ${test_points} (punctaj final)"    
            else
                total=$(bc <<< "$total + $test_points")
                echo "Test ${i}: OK        -  ${test_points} /  ${test_points} (punctaj final)"
            fi
          else
            echo "Test ${i}: WA        -    0 /  ${test_points}"
          fi
      fi

      rm -f ${EXE}.in ${EXE}.out
      echo ""
    done
}

function grade_homework()
{
    uname -a

    which gcc    
    if [ "$?" !=  "0" ]; then
        echo "gcc nu este instalat. STOP"
        display_score 1
    fi
    gcc --version | head -1

    which python    
    if [ "$?" !=  "0" ]; then
        echo "python nu este instalta. STOP"
        display_score 1
    fi
    python --version

    which valgrind    
    if [ "$?" !=  "0" ]; then
        echo "valgrind nu este instalat. STOP"
        display_score 1
    fi
    valgrind --version
    printf "\n"
        
    # check if Makefile exists
    if [ ! -f Makefile ]; then
        echo "Makefile lipsa. STOP"
        display_score 1
    fi

    # compile and check errors
    make -f Makefile build &> out.make
    if [ ! 0 -eq $? ]; then
        echo "Erori de compilare. STOP"

        cat out.make
        rm -f out.make
        
        display_score 1
    fi

    cnt=$(cat out.make | grep warning | wc -l)
    if [ $cnt -gt 0 ]; then
        total=$(bc <<< "$total - 5")
        echo "=======>>>>>> Ai warning-uri la compilare. Rusine! (-5p) <<<<<<======="

        cat out.make
        rm -f out.make

        echo ""
    else
        # just for info
        cat out.make
        rm -f out.make
    fi

    mkdir -p tests/out &> /dev/null

    # display tests set
    echo "=======================>>>>>> Run tests <<<<<< ======================="
    echo "Legenda:"
    echo "         BUBA      - programul NU se termina cu succes (SEGFAULT, TIMEOUT etc)"
    echo "         WA        - Wrong Answer - programul se termina cu succes, insa nu produce rezultatul dorit"
    echo "         OK        - programul se termina cu succes SI produce rezultatul dorit"
    echo "         MEM_ERR   - programul are erori de lucru cu memoria (ex. memory leaks, accese invalide la memorie)"
    echo ""
    echo ""

    # run tests
    test_homework

    # clean junk
    make -f Makefile clean &> /dev/null

    check_coding_style
    check_readme

    # display result
    display_score 0
}

if [ $# == 0 ]; then
    grade_homework "$@"
elif [ $# == 2 ] && [ $1 == "upload" ] && [ $2 == "vmchecker" ]; then
    ./upload.sh $EXE
else
    echo "Usage: ./checker"
fi
