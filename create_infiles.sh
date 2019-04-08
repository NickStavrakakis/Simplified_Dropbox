#!/bin/bash

dir_name=$1

num_of_files=$2
if ! [[ $2 =~ ^-?[0-9]+$ ]]; then
    echo "error: num_of_files is not a number"
    exit 1
fi

num_of_dirs=$3
if ! [[ $3 =~ ^-?[0-9]+$ ]]; then
    echo "error: num_of_dirs is not a number"
    exit 1
fi

if [[ $3 -lt 1 ]]; then
    echo "error: num_of_dirs is less than 1"
    exit 2
fi

levels=$4
if ! [[ $4 =~ ^-?[0-9]+$ ]]; then
    echo "error: levels is not a number"
    exit 1
fi

if [ ! -d $dir_name ]; then
    mkdir $dir_name
fi

while :
do
    #creating the total characters of the name of the new folder
    new_folder_chars=$(cat /dev/urandom | tr -dc '1-8' | fold -w 1 | head -n 1)
    #creating the name of the new folder
    new_folder_name=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $new_folder_chars | head -n 1)
    new_folder_name="$dir_name/$new_folder_name"
    mkdir $new_folder_name
    parent_name=$new_folder_name

    num_of_dirs="$(($num_of_dirs-1))"
    if [ "$num_of_dirs" -eq 0 ]; then
        break
    fi

    for (( j =1; j < levels ; j ++)) do

        #creating the total characters of the name of the new folder
        new_folder_chars=$(cat /dev/urandom | tr -dc '1-8' | fold -w 1 | head -n 1)
        #creating the name of the new folder
        new_folder_name=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $new_folder_chars | head -n 1)
        new_folder_name="$parent_name/$new_folder_name"
        mkdir $new_folder_name
        parent_name=$new_folder_name

        num_of_dirs="$(($num_of_dirs-1))"
        if [ "$num_of_dirs" -eq 0 ]; then
            break
        fi
    done
    if [ "$num_of_dirs" -eq 0 ]; then
        break
    fi
done

while :
do
    while read folder_name;
    do
        #creating the total characters of the name of the new file
        new_file_chars=$(cat /dev/urandom | tr -dc '1-8' | fold -w 1 | head -n 1)
        #creating the name of the new file
        new_file_name=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $new_file_chars | head -n 1)
        new_file_name="$folder_name/$new_file_name"

        #creating the total size of the new file (in KB)
        let "new_file_size = $RANDOM % 128 + 1"
        new_file_size+="K"

        #creating the new file
        base64 /dev/urandom | head -c $new_file_size > $new_file_name

        num_of_files="$(($num_of_files-1))"
        if [ "$num_of_files" -eq 0 ]; then
            break
        fi
    done < <(find $dir_name -type d)

    if [ "$num_of_files" -eq 0 ]; then
        break
    fi
done

exit 0
