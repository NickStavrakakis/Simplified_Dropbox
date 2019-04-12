#!/bin/bash
# client_id, s_bytes, r_bytes, s_files, r_files, deleted_flag
total_clients=0
total_s_bytes=0
total_r_bytes=0
total_s_files=0
total_r_files=0
total_deleted=0
while read log_file; do
	let "total_clients++"
	IFS=', ' read -r -a array <<< "$log_file"
	clients[$total_clients]=${array[0]}
	let "total_s_bytes+=${array[1]}"
	let "total_r_bytes+=${array[2]}"
	let "total_s_files+=${array[3]}"
	let "total_r_files+=${array[4]}"
	let "total_deleted+=${array[5]}"
done

TOTAL=${#clients[@]}
min_id=${array[0]}
max_id=${array[0]}

echo "Clients connected ($TOTAL): ${clients[@]}"

for ((i=1;i<=TOTAL;i++)); do
	if [ ${clients[$i]} -lt $min_id ]; then
		min_id=${clients[$i]}
	fi
	if [ ${clients[$i]} -gt $max_id ]; then
		max_id=${clients[$i]}
	fi
done

echo "Minimum id of clients: $min_id"
echo "Maximum id of clients: $max_id"
echo "Sended bytes: $total_s_bytes"
echo "Readed bytes: $total_r_bytes"
echo "Sended files: $total_s_files"
echo "Readed files: $total_r_files"
echo "Leaved clients: $total_deleted"
