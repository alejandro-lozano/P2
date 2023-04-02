#!/bin/bash

<<<<<<< HEAD
if [ $# != 1 ]; then
=======
if [ $# !=1 ]; then
>>>>>>> 82cfc6826bdbcf15d7fee4c2bef143326fd9660c
    echo "USAGE: $0 alfa0"
    exit -1
fi

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh

# Write here the name and path of your program and database
DIR_P2=$HOME/PAV/P2
DB=$DIR_P2/db.v4
CMD="$DIR_P2/bin/vad -0 $1"

for filewav in $DB/*/*wav; do
#    echo
    echo "**************** $filewav ****************" 
    if [[ ! -f $filewav ]]; then 
	    echo "Wav file not found: $filewav" >&2
	    exit 1
    fi

    filevad=${filewav/.wav/.vad} #sustituye el .wav por .vad

    $CMD -i $filewav -o $filevad || exit 1 
    #cortocircuito (OR) para exit,
    #si la primera  parte tiene éxito, ya no se ejecuta el exit

# Alternatively, uncomment to create output wave files
#    filewavOut=${filewav/.wav/.vad.wav}
#    $CMD $filewav $filevad $filewavOut || exit 1

done

scripts/vad_evaluation.pl $DB/*/*lab

exit 0
