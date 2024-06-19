#!/bin/bash
# GIACOMAZZI ANDREA

check_email(){
	if cut -d ',' -f4 address-book-database.csv | grep -q "^$1$"; 
	then
		return 0
	else
		return 1
	fi

}

if [[ $# < 1 ]];
then
	echo "Serve almeno un argomento!!"	
else 
	if [ $1 = "view" ];
	then
		if [[ $# -ne 1 ]];
		then
			echo "La funzione non ha argomenti"
		else
			column address-book-database.csv -t -s "," | (sed -u 1q; sort -k4) 
		fi
	elif [ $1 = "search" ];
	then
		if [[ $# -ne 2 ]];
		then
			echo "La funzione deve avere 1 argomento: la stringa da cercare"
		else
			string="$2"
			IFS=","
			found=false
			while read -r name surname phone mail city address;
			do
				echo "Name: $name"
				echo "Surname: $surname"
				echo "Phone: $phone"
				echo "Mail: $mail"
				echo "City: $city"
				echo "Address: $address"
				echo ""
				found=true
			done < <(grep "$string" address-book-database.csv)
			if ! $found; then
				echo "Not found"
			fi
		fi
	elif [ $1 = "insert" ];
	then
		if [[ $# -ne 1 ]];
		then
			echo "La funzione non ha argomenti"
		else
			read -p "Name: " name
			read -p "Surname: " surname
			read -p "Phone: " phone
			read -p "Mail: " mail
			read -p "City: " city
			read -p "Address: " address
			if check_email "$mail"; then
				echo "ERROR! Email already exists"
			else
				echo -e "\n$name,$surname,$phone,$mail,$city,$address" >> address-book-database.csv
				echo "Added"
			fi
		fi
	elif [ $1 = "delete" ];
	then
		if [[ $# -ne 2 ]];
		then
			echo "La funzione deve avere 1 argomento: l'e-mail da eliminare"
		else
			email="$2"
			if check_email "$email"; then
				csv_file="address-book-database.csv"
				#sed -i '/^$email,/d' address-book-database.csv
				temp_file=$(mktemp)
				awk -v string="$email" -F ',' '$4 != string' "$csv_file" > "$temp_file"
				mv "$temp_file" "$csv_file"
				rm -f "$temp_file"
				echo "Deleted"
			else
				echo "Cannot find any record"
			fi
		fi
	else
		echo "Nessuna funzione trovata"
	fi
fi




