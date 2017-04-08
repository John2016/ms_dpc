#!/bin/sh

your_name="liyuan"


for file in 'ls /etc'

echo $your_name
echo ${your_name}

for skill in Ada Coffe Action Java; do
	echo "I am good ar ${skill}Script"
done

#
#

{


}

greeting="hello! "$your_name" !"
echo $(#greeting)  	# length of string
echo $(string:1:4)	# substring, count from 0, 1 and 4 are included
echo 'expr index "$string" is'	# index of is in string

#---------------------------------------------
if condition
	then
		command		# necessary
	elif [[ condition ]]; 
		command
	else
		command
fi

#---------------------------------------------
for var in item1 item2 item3 
do
	command
done

#---------------------------------------------
while condition
do
	command
done

#---------------------------------------------
while :		# or while true
do
	command
done




