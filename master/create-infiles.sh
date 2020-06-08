#!/bin/bash
# DiseaseAggregator script

dirCreation(){
  dirName=$1
  if [ ! -d "$dirName" ]
  then
    echo "Directory doesn't exist. Creating Now.."
    mkdir -p $dirName
    echo "Directory $dirName created."
  else
    echo "Directory $dirName already exists."
  fi
}

checkNums(){
  number=$1
  if [[ ! $number == ?(-)+([0-9]) ]]
    then
      echo "ERROR: 4rd and 5rd argument must be a number."
      exit 1
  fi
  if [[ $number -le 0 ]]
    then
      echo "ERROR: Numbers must be above zero."
      exit 1
  fi
}

diseaseFile=$1
countriesFile=$2
input_dir=$3
numFilesPerDirectory=$4
numRecordsPerFile=$5

if [ ! $# -eq 5 ]
  then
    echo "Wrong number of arguments supplied."
    exit 1
fi
checkNums $numFilesPerDirectory
checkNums $numRecordsPerFile
readarray -t diseases <$diseaseFile
readarray -t countries <$countriesFile
dirCreation $input_dir
ascID=1
declare -a enteredRecords
for i in ${countries[@]}
do
  dirCreation "$input_dir/$i"
  unset enteredRecords
  for j in `seq $numFilesPerDirectory`
  do
    fileName=`date -d "2020-01-01 $j days" +$input_dir/$i/%d-%m-%Y.txt`
    touch $fileName
    for k in $(seq $numRecordsPerFile)
    do
      firstName=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w 10 | head -n 1)
      lastName=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w 10 | head -n 1)
      rndDisease=${diseases[$RANDOM % ${#diseases[@]} ]}
      age=$(( ( RANDOM % 120 )  + 1 ))
      enrtyORexit=$(( ( RANDOM % 100 )  + 1 ))
      if [[ $enrtyORexit -le 10 ]]
      then
        status="EXIT"
        record="$ascID $status $firstName $lastName $rndDisease $age"
        echo "$record" >> $fileName
        ascID=$(( $ascID + 1 ))
      elif [[ $enrtyORexit -le 55 ]]
      then
        status="EXIT"
        if [ ${#enteredRecords[@]} -eq 0 ]
        then
          status="ENTRY"
          record="$ascID $status $firstName $lastName $rndDisease $age"
          echo "$record" >> $fileName
          enteredRecords+=("$ascID EXIT $firstName $lastName $rndDisease $age")
          ascID=$(( $ascID + 1 ))
        else
          echo "${enteredRecords[0]}" >> $fileName
          enteredRecords=("${enteredRecords[@]:1}")
        fi
      else
        status="ENTRY"
        record="$ascID $status $firstName $lastName $rndDisease $age"
        echo "$record" >> $fileName
        enteredRecords+=("$ascID EXIT $firstName $lastName $rndDisease $age")
        ascID=$(( $ascID + 1 ))
      fi
    done
  done
done